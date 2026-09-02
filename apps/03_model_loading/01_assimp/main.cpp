/**
 * @file main.cpp
 * @brief 使用 Assimp 导入 OBJ 模型并渲染场景数据。
 *
 * @details
 * 手写顶点数组只适合立方体这类规则几何，真实模型通常由建模软件导出。
 * Assimp（Open Asset Import Library）把 OBJ/FBX/glTF 等几十种格式统一解析成
 * aiScene 数据结构：场景由 aiNode 节点树组织，每个节点引用若干 aiMesh，
 * mesh 再通过材质索引关联 aiMaterial。
 * 本示例加载 assets/models/crate/crate.obj，打印场景统计信息，
 * 把每个 mesh 上传到独立的 VAO/EBO，然后沿节点树递归渲染，
 * 用"法线 -> RGB"着色直观展示几何朝向（不涉及纹理和光照）。
 */

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {

constexpr int window_width{800};
constexpr int window_height{600};
constexpr const char* window_title{"OpenGL Lab - Assimp Import"};

#if !defined(OPENGL_LAB_ASSET_ROOT)
#define OPENGL_LAB_ASSET_ROOT "."
#endif

constexpr const char* vertex_shader_source{R"glsl(
#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

out vec3 normal_world;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 模型只做刚体旋转，用 model 矩阵直接变换法线即可（无非均匀缩放）。
    normal_world = normalize(mat3(model) * a_normal);
    gl_Position = projection * view * model * vec4(a_pos, 1.0);
}
)glsl"};

constexpr const char* fragment_shader_source{R"glsl(
#version 330 core
in vec3 normal_world;
out vec4 frag_color;

void main()
{
    // 法线可视化：把 [-1, 1] 的法线分量映射到 [0, 1] 的 RGB，直观展示几何朝向。
    frag_color = vec4(normalize(normal_world) * 0.5 + 0.5, 1.0);
}
)glsl"};

/**
 * @brief 单个 aiMesh 对应的 GPU 资源句柄。
 *
 * @details
 * VAO 记录 attribute 布局，VBO 保存交错顶点数据，EBO 保存索引。
 * index_count 是绘制一个 mesh 需要的索引数量。
 */
struct GpuMesh {
    GLuint vertex_array_object{0};
    GLuint vertex_buffer_object{0};
    GLuint element_buffer_object{0};
    GLsizei index_count{0};
};

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
 * @brief 生成模型文件路径。
 */
std::string model_path(const char* relative_path) {
    std::filesystem::path path{OPENGL_LAB_ASSET_ROOT};
    path /= "models";
    path /= relative_path;
    return path.generic_string();
}

/**
 * @brief 把 Assimp 的行主序矩阵转换成 GLM 的列主序矩阵。
 *
 * @details
 * aiMatrix4x4 按行主序存储（a1..a4 是第一行），glm::mat4 构造函数按列主序接收参数，
 * 转换时必须行列互换，否则节点变换会被转置，模型会出现镜像或错位。
 */
glm::mat4 to_glm_mat4(const aiMatrix4x4& transform) {
    return glm::mat4{transform.a1, transform.b1, transform.c1, transform.d1,
                     transform.a2, transform.b2, transform.c2, transform.d2,
                     transform.a3, transform.b3, transform.c3, transform.d3,
                     transform.a4, transform.b4, transform.c4, transform.d4};
}

/**
 * @brief 打印 aiScene 的结构统计，帮助理解 Assimp 的数据模型。
 */
void print_scene_stats(const aiScene* scene) {
    std::cout << "scene: " << scene->mNumMeshes << " mesh(es), " << scene->mNumMaterials
              << " material(s)\n";

    for (unsigned int index{0U}; index < scene->mNumMeshes; ++index) {
        const aiMesh* mesh{scene->mMeshes[index]};
        std::cout << "  mesh[" << index << "] '" << mesh->mName.C_Str()
                  << "': " << mesh->mNumVertices << " vertices, " << mesh->mNumFaces
                  << " faces, material " << mesh->mMaterialIndex
                  << ", normals=" << (mesh->mNormals != nullptr ? "yes" : "no")
                  << ", uv=" << (mesh->mTextureCoords[0] != nullptr ? "yes" : "no") << '\n';
    }

    for (unsigned int index{0U}; index < scene->mNumMaterials; ++index) {
        const aiMaterial* material{scene->mMaterials[index]};
        aiString material_name{};
        material->Get(AI_MATKEY_NAME, material_name);

        aiString texture_path{};
        const unsigned int diffuse_count{material->GetTextureCount(aiTextureType_DIFFUSE)};
        if (diffuse_count > 0U) {
            material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_path);
        }
        std::cout << "  material[" << index << "] '" << material_name.C_Str()
                  << "': diffuse maps=" << diffuse_count;
        if (diffuse_count > 0U) {
            std::cout << " (first: " << texture_path.C_Str() << ")";
        }
        std::cout << '\n';
    }
}

/**
 * @brief 把 aiMesh 的顶点和索引数据上传到 GPU。
 *
 * @details
 * 位置/法线/纹理坐标交错写入同一块 VBO，attribute 布局与 02_lighting 章节一致
 * （0 = 位置，1 = 法线，2 = 纹理坐标），渲染端因此能复用同一套解释逻辑。
 * 法线和 UV 缺失时补零：aiProcess_GenSmoothNormals 通常已经保证法线存在，
 * 这里仍然防御式处理，避免把空指针解引用进顶点数据。
 */
GpuMesh upload_mesh(const aiMesh* mesh) {
    std::vector<float> vertex_data;
    vertex_data.reserve(static_cast<std::size_t>(mesh->mNumVertices) * 8U);

    for (unsigned int index{0U}; index < mesh->mNumVertices; ++index) {
        const aiVector3D& position{mesh->mVertices[index]};
        vertex_data.push_back(position.x);
        vertex_data.push_back(position.y);
        vertex_data.push_back(position.z);

        if (mesh->mNormals != nullptr) {
            const aiVector3D& normal{mesh->mNormals[index]};
            vertex_data.push_back(normal.x);
            vertex_data.push_back(normal.y);
            vertex_data.push_back(normal.z);
        } else {
            vertex_data.insert(vertex_data.end(), 3, 0.0F);
        }

        if (mesh->mTextureCoords[0] != nullptr) {
            const aiVector3D& tex_coord{mesh->mTextureCoords[0][index]};
            vertex_data.push_back(tex_coord.x);
            vertex_data.push_back(tex_coord.y);
        } else {
            vertex_data.insert(vertex_data.end(), 2, 0.0F);
        }
    }

    // Assimp: mFaces 里的索引是 mesh 顶点数组的下标，triangulate 后每个面恰好 3 个。
    std::vector<GLuint> indices;
    indices.reserve(static_cast<std::size_t>(mesh->mNumFaces) * 3U);
    for (unsigned int face_index{0U}; face_index < mesh->mNumFaces; ++face_index) {
        const aiFace& face{mesh->mFaces[face_index]};
        for (unsigned int corner{0U}; corner < face.mNumIndices; ++corner) {
            indices.push_back(face.mIndices[corner]);
        }
    }

    GpuMesh gpu_mesh{};
    glGenVertexArrays(1, &gpu_mesh.vertex_array_object);
    glGenBuffers(1, &gpu_mesh.vertex_buffer_object);
    glGenBuffers(1, &gpu_mesh.element_buffer_object);

    glBindVertexArray(gpu_mesh.vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, gpu_mesh.vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertex_data.size() * sizeof(float)),
                 vertex_data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpu_mesh.element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(GLuint)),
                 indices.data(), GL_STATIC_DRAW);

    constexpr GLsizei vertex_stride{8 * static_cast<GLsizei>(sizeof(float))};
    constexpr auto normal_offset{3 * sizeof(float)};
    constexpr auto texture_offset{6 * sizeof(float)};

    // OpenGL: attribute 0 解释顶点位置。
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_stride, nullptr);
    glEnableVertexAttribArray(0);
    // OpenGL: attribute 1 解释法线，偏移 3 个 float。
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_stride,
                          reinterpret_cast<const void*>(normal_offset));
    glEnableVertexAttribArray(1);
    // OpenGL: attribute 2 解释纹理坐标，偏移 6 个 float。
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertex_stride,
                          reinterpret_cast<const void*>(texture_offset));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    gpu_mesh.index_count = static_cast<GLsizei>(indices.size());
    return gpu_mesh;
}

/**
 * @brief 沿 aiNode 节点树递归渲染节点引用的所有 mesh。
 *
 * @param node 当前节点。
 * @param parent_transform 父节点累积变换（世界空间）。
 * @param shader_program 渲染使用的 program。
 * @param gpu_meshes 按场景 mesh 下标索引的 GPU 资源数组。
 */
void render_node(const aiNode* node, const glm::mat4& parent_transform, GLuint shader_program,
                 const std::vector<GpuMesh>& gpu_meshes) {
    // Assimp: 节点变换相对父节点，左乘父级累积变换得到世界变换。
    const glm::mat4 node_transform{to_glm_mat4(node->mTransformation) * parent_transform};

    for (unsigned int mesh_slot{0U}; mesh_slot < node->mNumMeshes; ++mesh_slot) {
        const unsigned int scene_mesh_index{node->mMeshes[mesh_slot]};
        const GpuMesh& gpu_mesh{gpu_meshes[scene_mesh_index]};

        glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE,
                           glm::value_ptr(node_transform));
        glBindVertexArray(gpu_mesh.vertex_array_object);
        glDrawElements(GL_TRIANGLES, gpu_mesh.index_count, GL_UNSIGNED_INT, nullptr);
    }

    for (unsigned int child{0U}; child < node->mNumChildren; ++child) {
        render_node(node->mChildren[child], node_transform, shader_program, gpu_meshes);
    }
}

}  // namespace

/**
 * @brief assimp import 示例入口。
 *
 * @return 正常运行返回 EXIT_SUCCESS；初始化、导入或编译失败返回 EXIT_FAILURE。
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

    const GLuint shader_program{
        create_shader_program(vertex_shader_source, fragment_shader_source)};
    if (shader_program == 0U) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Assimp: Importer 负责解析并拥有 aiScene，析构时自动释放，不需要手动 delete。
    Assimp::Importer importer;

    // Assimp: 后处理把导入数据整理成 GPU 友好的形式——
    // Triangulate 把多边形拆成三角形；GenSmoothNormals 为缺法线的 mesh 生成平滑法线
    // （已有法线则保持原样）；JoinIdenticalVertices 合并重复顶点；OptimizeMeshes
    // 把过碎的 mesh 合并减少绘制调用。注意不使用 aiProcess_FlipUVs：本仓库纹理
    // 统一由 stbi_set_flip_vertically_on_load(1) 翻转，后处理再翻会导致二次翻转。
    constexpr unsigned int import_flags{aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                        aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes};
    const aiScene* scene{importer.ReadFile(model_path("crate/crate.obj"), import_flags)};
    if (scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U ||
        scene->mRootNode == nullptr) {
        std::cerr << "Failed to import model: " << importer.GetErrorString() << '\n';
        glDeleteProgram(shader_program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    print_scene_stats(scene);

    // 场景 mesh 只上传一次；同一 mesh 被多个节点引用时（实例化）直接复用 GPU 资源。
    std::vector<GpuMesh> gpu_meshes{};

    for (unsigned int index{0U}; index < scene->mNumMeshes; ++index) {
        gpu_meshes.push_back(upload_mesh(scene->mMeshes[index]));
    }

    const glm::mat4 view{glm::lookAt(glm::vec3{1.6F, 1.1F, 2.3F}, glm::vec3{0.0F, 0.0F, 0.0F},
                                     glm::vec3{0.0F, 1.0F, 0.0F})};
    const glm::mat4 projection{glm::perspective(
        glm::radians(45.0F), static_cast<float>(window_width) / static_cast<float>(window_height),
        0.1F, 100.0F)};

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        glClearColor(0.08F, 0.10F, 0.14F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader_program);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));

        // Camera: 相机固定，模型绕 Y 轴自转，从各侧面观察法线着色。
        glm::mat4 model{1.0F};
        model = glm::rotate(model, glm::radians(static_cast<float>(glfwGetTime()) * 25.0F),
                            glm::vec3{0.0F, 1.0F, 0.0F});
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));

        render_node(scene->mRootNode, glm::mat4{1.0F}, shader_program, gpu_meshes);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (GpuMesh& gpu_mesh : gpu_meshes) {
        glDeleteBuffers(1, &gpu_mesh.element_buffer_object);
        glDeleteBuffers(1, &gpu_mesh.vertex_buffer_object);
        glDeleteVertexArrays(1, &gpu_mesh.vertex_array_object);
    }

    glDeleteProgram(shader_program);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
