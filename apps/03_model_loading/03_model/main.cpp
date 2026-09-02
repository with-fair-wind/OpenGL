/**
 * @file main.cpp
 * @brief Model 封装：递归加载多 mesh、多材质模型并渲染。
 *
 * @details
 * Mesh 解决了"一份几何怎么画"，Model 解决"一个模型文件怎么装进一堆 Mesh"：
 * Assimp 解析出 aiScene 后，Model 沿 aiNode 节点树递归遍历，把每个节点引用的
 * aiMesh 转换成 Mesh（顶点/索引进 GPU），同时从 aiMaterial 读取贴图路径、
 * 在模型目录下加载纹理，缺少贴图时回退到 1x1 纯色纹理。
 * 本示例加载 assets/models/stage/stage.obj（地面/箱子/八面体三种材质），
 * 用一个环绕运动的点光源照亮，验证多 mesh、多材质、贴图回退三条路径。
 */

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb_image.h>

namespace {

constexpr int window_width{800};
constexpr int window_height{600};
constexpr const char* window_title{"OpenGL Lab - Model Loading"};

#if !defined(OPENGL_LAB_ASSET_ROOT)
#define OPENGL_LAB_ASSET_ROOT "."
#endif

const glm::vec3 view_position{3.0F, 2.4F, 3.4F};
const glm::vec3 view_target{0.0F, 0.45F, 0.0F};

constexpr const char* object_vertex_shader_source{R"glsl(
#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coord;

out vec3 frag_pos;
out vec3 normal;
out vec2 tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    frag_pos = vec3(model * vec4(a_pos, 1.0));
    normal = mat3(transpose(inverse(model))) * a_normal;
    tex_coord = a_tex_coord;
    gl_Position = projection * view * vec4(frag_pos, 1.0);
}
)glsl"};

constexpr const char* object_fragment_shader_source{R"glsl(
#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 frag_pos;
in vec3 normal;
in vec2 tex_coord;
out vec4 frag_color;

uniform vec3 view_pos;
uniform Material material;
uniform PointLight light;

void main()
{
    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light.position - frag_pos);

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(normalize(view_pos - frag_pos), reflect_dir), 0.0),
                     material.shininess);

    vec3 diffuse_sample = vec3(texture(material.diffuse, tex_coord));
    vec3 specular_sample = vec3(texture(material.specular, tex_coord));
    vec3 ambient = light.ambient * diffuse_sample;
    vec3 diffuse = light.diffuse * diff * diffuse_sample;
    vec3 specular = light.specular * spec * specular_sample;

    frag_color = vec4(ambient + diffuse + specular, 1.0);
}
)glsl"};

constexpr const char* light_vertex_shader_source{R"glsl(
#version 330 core
layout (location = 0) in vec3 a_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(a_pos, 1.0);
}
)glsl"};

constexpr const char* light_fragment_shader_source{R"glsl(
#version 330 core
out vec4 frag_color;
uniform vec3 light_color;

void main()
{
    frag_color = vec4(light_color, 1.0);
}
)glsl"};

/**
 * @brief 顶点数据的最小单元：位置、法线、纹理坐标。
 */
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

/**
 * @brief Mesh：一份几何与材质纹理的 GPU 封装（与 02_mesh 示例一致）。
 */
struct Mesh {
    GLuint vertex_array_object{0};
    GLuint vertex_buffer_object{0};
    GLuint element_buffer_object{0};
    GLsizei index_count{0};
    GLuint diffuse_map{0};
    GLuint specular_map{0};
    float shininess{32.0F};
};

/**
 * @brief Model：一个模型文件加载后的全部 Mesh。
 *
 * @details
 * directory 是模型文件所在目录，材质里的贴图路径都相对它解析，
 * 与常见"模型和贴图放在同一目录"的资源组织方式一致。
 */
struct Model {
    std::vector<Mesh> meshes;
    std::string directory;
};

/**
 * @brief stb_image 图片内存释放器。
 */
struct stbi_image_deleter {
    void operator()(stbi_uc* data) const noexcept {
        stbi_image_free(data);
    }
};

using stbi_image_ptr = std::unique_ptr<stbi_uc, stbi_image_deleter>;

/**
 * @brief 回调：窗口帧缓冲尺寸变化时更新 OpenGL 视口。
 */
void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    // OpenGL: 默认帧缓冲尺寸改变后必须更新视口，否则画面会被裁切或拉伸。
    glViewport(0, 0, width, height);
}

/**
 * @brief 处理 ESC 退出输入。
 */
void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

/**
 * @brief 编译指定阶段的 GLSL shader。
 */
GLuint compile_shader(GLenum shader_type, const char* source) {
    const GLuint shader{glCreateShader(shader_type)};
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success{GL_FALSE};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) {
        return shader;
    }

    std::array<char, 1024> info_log{};
    glGetShaderInfoLog(shader, static_cast<GLsizei>(info_log.size()), nullptr, info_log.data());
    std::cerr << "Failed to compile shader:\n" << info_log.data() << '\n';
    glDeleteShader(shader);
    return 0U;
}

/**
 * @brief 链接顶点着色器和片段着色器。
 */
GLuint create_shader_program(const char* vertex_source, const char* fragment_source) {
    const GLuint vertex_shader{compile_shader(GL_VERTEX_SHADER, vertex_source)};
    if (vertex_shader == 0U) {
        return 0U;
    }

    const GLuint fragment_shader{compile_shader(GL_FRAGMENT_SHADER, fragment_source)};
    if (fragment_shader == 0U) {
        glDeleteShader(vertex_shader);
        return 0U;
    }

    const GLuint shader_program{glCreateProgram()};
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    GLint success{GL_FALSE};
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (success == GL_TRUE) {
        return shader_program;
    }

    std::array<char, 1024> info_log{};
    glGetProgramInfoLog(shader_program, static_cast<GLsizei>(info_log.size()), nullptr,
                        info_log.data());
    std::cerr << "Failed to link shader program:\n" << info_log.data() << '\n';
    glDeleteProgram(shader_program);
    return 0U;
}

/**
 * @brief 从完整路径加载 2D 纹理。
 *
 * @return 纹理对象句柄；文件不存在或解码失败返回 0。
 */
GLuint create_texture_from_file(const std::string& path) {
    int width{0};
    int height{0};
    int channel_count{0};

    stbi_set_flip_vertically_on_load(1);
    stbi_image_ptr image_data{
        stbi_load(path.c_str(), &width, &height, &channel_count, 0),
    };
    if (image_data == nullptr) {
        std::cerr << "Failed to load texture: " << path << '\n';
        return 0U;
    }

    GLenum format{GL_RGB};
    if (channel_count == 1) {
        format = GL_RED;
    } else if (channel_count == 4) {
        format = GL_RGBA;
    }

    GLuint texture{0};
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0, format,
                 GL_UNSIGNED_BYTE, image_data.get());
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

/**
 * @brief 生成 1x1 纯色纹理，作为缺失贴图的回退。
 *
 * @details
 * 无贴图材质（如纯色的八面体）回退到单色纹理后，着色器代码无需分支——
 * 采样结果恒为该颜色。1x1 纹理没有多级 mip，MIN_FILTER 必须用 GL_LINEAR；
 * 若保留 GL_LINEAR_MIPMAP_LINEAR，纹理会因 mipmap 链不完整而采样为黑。
 */
GLuint create_solid_texture(const glm::vec3& color) {
    const std::array<unsigned char, 3> pixels{
        static_cast<unsigned char>(color.r * 255.0F + 0.5F),
        static_cast<unsigned char>(color.g * 255.0F + 0.5F),
        static_cast<unsigned char>(color.b * 255.0F + 0.5F),
    };

    GLuint texture{0};
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    return texture;
}

/**
 * @brief 把顶点数组和索引数组上传到 GPU，生成一个 Mesh（与 02_mesh 示例一致）。
 */
Mesh make_mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices,
               GLuint diffuse_map, GLuint specular_map, float shininess) {
    Mesh mesh{};
    mesh.diffuse_map = diffuse_map;
    mesh.specular_map = specular_map;
    mesh.shininess = shininess;
    mesh.index_count = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &mesh.vertex_array_object);
    glGenBuffers(1, &mesh.vertex_buffer_object);
    glGenBuffers(1, &mesh.element_buffer_object);

    glBindVertexArray(mesh.vertex_array_object);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(GLuint)),
                 indices.data(), GL_STATIC_DRAW);

    constexpr GLsizei vertex_stride{static_cast<GLsizei>(sizeof(Vertex))};
    constexpr auto normal_offset{3 * sizeof(float)};
    constexpr auto tex_coord_offset{6 * sizeof(float)};
    static_assert(sizeof(Vertex) == 8 * sizeof(float), "Vertex must stay tightly packed");

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_stride,
                          reinterpret_cast<const void*>(normal_offset));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertex_stride,
                          reinterpret_cast<const void*>(tex_coord_offset));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return mesh;
}

/**
 * @brief 绘制一个 Mesh。
 */
void draw_mesh(const Mesh& mesh) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mesh.diffuse_map);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mesh.specular_map);

    glBindVertexArray(mesh.vertex_array_object);
    glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

/**
 * @brief 把 aiMesh 及其材质转换成 Mesh。
 *
 * @details
 * 顶点部分与 01_assimp 示例相同；材质部分读取 aiMaterial：
 * - 贴图路径相对模型目录解析（OBJ 的 map_Kd/map_Ks 写的是相对文件名）；
 * - 缺贴图时回退——漫反射用材质的 Kd 纯色，镜面用黑色（无高光）；
 * - shininess 取 MTL 的 Ns（高光指数），缺失时用 32。
 */
Mesh process_mesh(const Model& model, const aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);

    for (unsigned int index{0U}; index < mesh->mNumVertices; ++index) {
        Vertex vertex{};
        vertex.position =
            glm::vec3{mesh->mVertices[index].x, mesh->mVertices[index].y, mesh->mVertices[index].z};

        // Assimp: GenSmoothNormals 已保证法线存在，防御式判空避免解引用空指针。
        if (mesh->mNormals != nullptr) {
            const aiVector3D& normal{mesh->mNormals[index]};
            vertex.normal = glm::vec3{normal.x, normal.y, normal.z};
        }

        // Assimp: 一个 mesh 可有多组 UV，OpenGL 侧只取第 0 组；缺 UV 时保持 (0, 0)。
        if (mesh->mTextureCoords[0] != nullptr) {
            const aiVector3D& tex_coord{mesh->mTextureCoords[0][index]};
            vertex.tex_coords = glm::vec2{tex_coord.x, tex_coord.y};
        }
        vertices.push_back(vertex);
    }

    std::vector<GLuint> indices;
    indices.reserve(static_cast<std::size_t>(mesh->mNumFaces) * 3U);
    for (unsigned int face_index{0U}; face_index < mesh->mNumFaces; ++face_index) {
        const aiFace& face{mesh->mFaces[face_index]};
        for (unsigned int corner{0U}; corner < face.mNumIndices; ++corner) {
            indices.push_back(face.mIndices[corner]);
        }
    }

    const aiMaterial* material{scene->mMaterials[mesh->mMaterialIndex]};

    GLuint diffuse_map{0};
    aiString texture_path{};
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0U &&
        material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path) == aiReturn_SUCCESS) {
        std::filesystem::path texture_file{model.directory};
        texture_file /= texture_path.C_Str();
        diffuse_map = create_texture_from_file(texture_file.generic_string());
    }

    GLuint specular_map{0};
    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0U &&
        material->GetTexture(aiTextureType_SPECULAR, 0, &texture_path) == aiReturn_SUCCESS) {
        std::filesystem::path texture_file{model.directory};
        texture_file /= texture_path.C_Str();
        specular_map = create_texture_from_file(texture_file.generic_string());
    }

    // 材质回退：漫反射缺贴图用 Kd 纯色，镜面缺贴图用黑色（该 mesh 无高光）。
    aiColor3D diffuse_color{1.0F, 1.0F, 1.0F};
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);
    if (diffuse_map == 0U) {
        std::cout << "  mesh '" << mesh->mName.C_Str()
                  << "': no diffuse map, fallback to material color\n";
        diffuse_map =
            create_solid_texture(glm::vec3{diffuse_color.r, diffuse_color.g, diffuse_color.b});
    }
    if (specular_map == 0U) {
        specular_map = create_solid_texture(glm::vec3{0.0F, 0.0F, 0.0F});
    }

    float shininess{32.0F};
    if (material->Get(AI_MATKEY_SHININESS, shininess) != aiReturn_SUCCESS) {
        shininess = 32.0F;
    }

    return make_mesh(vertices, indices, diffuse_map, specular_map, shininess);
}

/**
 * @brief 递归处理节点：转换当前节点的每个 mesh，再深入子节点。
 */
void process_node(Model& model, const aiNode* node, const aiScene* scene) {
    for (unsigned int mesh_slot{0U}; mesh_slot < node->mNumMeshes; ++mesh_slot) {
        const aiMesh* mesh{scene->mMeshes[node->mMeshes[mesh_slot]]};
        model.meshes.push_back(process_mesh(model, mesh, scene));
    }

    for (unsigned int child{0U}; child < node->mNumChildren; ++child) {
        process_node(model, node->mChildren[child], scene);
    }
}

/**
 * @brief 加载模型文件，填充 model.meshes。
 *
 * @return 导入并处理成功返回 true；文件损坏或场景不完整返回 false。
 */
bool load_model(Model& model, const std::filesystem::path& path) {
    Assimp::Importer importer;

    // Assimp: 后处理与 01_assimp 相同；不使用 aiProcess_FlipUVs，
    // 因为纹理统一由 stbi_set_flip_vertically_on_load(1) 翻转，后处理再翻会二次翻转。
    constexpr unsigned int import_flags{aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                        aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes};
    const aiScene* scene{importer.ReadFile(path.generic_string(), import_flags)};
    if (scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U ||
        scene->mRootNode == nullptr) {
        std::cerr << "Failed to import model: " << importer.GetErrorString() << '\n';
        return false;
    }

    model.directory = path.parent_path().generic_string();
    model.meshes.clear();
    process_node(model, scene->mRootNode, scene);

    std::cout << "loaded '" << path.filename().generic_string() << "': " << model.meshes.size()
              << " mesh(es)\n";
    return !model.meshes.empty();
}

/**
 * @brief 释放 Model 持有的全部 GPU 对象。
 *
 * @pre 仍在 OpenGL 上下文存活期间调用（先于 glfwTerminate）。
 */
void destroy_model(Model& model) {
    for (Mesh& mesh : model.meshes) {
        glDeleteTextures(1, &mesh.specular_map);
        glDeleteTextures(1, &mesh.diffuse_map);
        glDeleteBuffers(1, &mesh.element_buffer_object);
        glDeleteBuffers(1, &mesh.vertex_buffer_object);
        glDeleteVertexArrays(1, &mesh.vertex_array_object);
    }
    model.meshes.clear();
}

}  // namespace

/**
 * @brief model 示例入口。
 *
 * @return 正常运行返回 EXIT_SUCCESS；初始化、编译或模型加载失败返回 EXIT_FAILURE。
 */
int main() {
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window =
        glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glViewport(0, 0, window_width, window_height);
    glEnable(GL_DEPTH_TEST);

    const GLuint object_program{
        create_shader_program(object_vertex_shader_source, object_fragment_shader_source)};
    const GLuint light_program{
        create_shader_program(light_vertex_shader_source, light_fragment_shader_source)};
    if (object_program == 0U || light_program == 0U) {
        glDeleteProgram(light_program);
        glDeleteProgram(object_program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::filesystem::path model_file{OPENGL_LAB_ASSET_ROOT};
    model_file /= "models";
    model_file /= "stage";
    model_file /= "stage.obj";

    Model stage{};
    if (!load_model(stage, model_file)) {
        glDeleteProgram(light_program);
        glDeleteProgram(object_program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // 光源小立方体：只含位置的 36 顶点立方体，用于可视化点光源位置。
    constexpr std::array<float, 108> light_cube_vertices{
        // -Z 面（后）
        -0.5F,
        -0.5F,
        -0.5F,
        0.5F,
        -0.5F,
        -0.5F,
        0.5F,
        0.5F,
        -0.5F,
        0.5F,
        0.5F,
        -0.5F,
        -0.5F,
        0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        // +Z 面（前）
        -0.5F,
        -0.5F,
        0.5F,
        0.5F,
        -0.5F,
        0.5F,
        0.5F,
        0.5F,
        0.5F,
        0.5F,
        0.5F,
        0.5F,
        -0.5F,
        0.5F,
        0.5F,
        -0.5F,
        -0.5F,
        0.5F,
        // -X 面（左）
        -0.5F,
        0.5F,
        0.5F,
        -0.5F,
        0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        0.5F,
        -0.5F,
        0.5F,
        0.5F,
        // +X 面（右）
        0.5F,
        0.5F,
        0.5F,
        0.5F,
        0.5F,
        -0.5F,
        0.5F,
        -0.5F,
        -0.5F,
        0.5F,
        -0.5F,
        -0.5F,
        0.5F,
        -0.5F,
        0.5F,
        0.5F,
        0.5F,
        0.5F,
        // -Y 面（下）
        -0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        0.5F,
        0.5F,
        -0.5F,
        0.5F,
        0.5F,
        -0.5F,
        0.5F,
        0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        -0.5F,
        // +Y 面（上）
        -0.5F,
        0.5F,
        -0.5F,
        0.5F,
        0.5F,
        -0.5F,
        0.5F,
        0.5F,
        0.5F,
        0.5F,
        0.5F,
        0.5F,
        -0.5F,
        0.5F,
        0.5F,
        -0.5F,
        0.5F,
        -0.5F,
    };

    GLuint light_vao{0};
    GLuint light_vbo{0};
    glGenVertexArrays(1, &light_vao);
    glGenBuffers(1, &light_vbo);

    glBindVertexArray(light_vao);
    glBindBuffer(GL_ARRAY_BUFFER, light_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(light_cube_vertices.size() * sizeof(float)),
                 light_cube_vertices.data(), GL_STATIC_DRAW);

    // OpenGL: 光源立方体只用位置 attribute 0。
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * static_cast<GLsizei>(sizeof(float)),
                          nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    const glm::mat4 view{glm::lookAt(view_position, view_target, glm::vec3{0.0F, 1.0F, 0.0F})};
    const glm::mat4 projection{glm::perspective(
        glm::radians(45.0F), static_cast<float>(window_width) / static_cast<float>(window_height),
        0.1F, 100.0F)};

    glUseProgram(object_program);
    glUniform1i(glGetUniformLocation(object_program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(object_program, "material.specular"), 1);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        glClearColor(0.03F, 0.04F, 0.07F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera: 点光源绕场景中心环绕运动，让三种材质轮流正对光照。
        const float time{static_cast<float>(glfwGetTime())};
        const glm::vec3 light_position{std::cos(time * 0.7F) * 2.4F, 2.2F,
                                       std::sin(time * 0.7F) * 2.4F};

        glUseProgram(object_program);
        glUniform3fv(glGetUniformLocation(object_program, "view_pos"), 1,
                     glm::value_ptr(view_position));
        glUniform3fv(glGetUniformLocation(object_program, "light.position"), 1,
                     glm::value_ptr(light_position));
        glUniform3f(glGetUniformLocation(object_program, "light.ambient"), 0.15F, 0.15F, 0.15F);
        glUniform3f(glGetUniformLocation(object_program, "light.diffuse"), 0.90F, 0.90F, 0.90F);
        glUniform3f(glGetUniformLocation(object_program, "light.specular"), 1.0F, 1.0F, 1.0F);
        glUniformMatrix4fv(glGetUniformLocation(object_program, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(object_program, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));

        // Assimp: stage.obj 的顶点已是世界坐标，节点树变换为单位阵，model 保持恒等。
        const glm::mat4 model{1.0F};
        glUniformMatrix4fv(glGetUniformLocation(object_program, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));

        // Model: 每个 mesh 携带自己的材质（贴图 + shininess），逐个设置并绘制。
        for (const Mesh& mesh : stage.meshes) {
            glUniform1f(glGetUniformLocation(object_program, "material.shininess"), mesh.shininess);
            draw_mesh(mesh);
        }

        glUseProgram(light_program);
        glUniform3f(glGetUniformLocation(light_program, "light_color"), 1.0F, 1.0F, 1.0F);
        glUniformMatrix4fv(glGetUniformLocation(light_program, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(light_program, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));

        glm::mat4 light_model{1.0F};
        light_model = glm::translate(light_model, light_position);
        light_model = glm::scale(light_model, glm::vec3{0.08F});
        glUniformMatrix4fv(glGetUniformLocation(light_program, "model"), 1, GL_FALSE,
                           glm::value_ptr(light_model));
        glBindVertexArray(light_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &light_vbo);
    glDeleteVertexArrays(1, &light_vao);
    destroy_model(stage);
    glDeleteProgram(light_program);
    glDeleteProgram(object_program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
