/**
 * @file main.cpp
 * @brief Mesh 封装：把顶点、索引和材质纹理打包成可复用的绘制单元。
 *
 * @details
 * 前两章每个示例都手写 VAO/VBO/EBO 和 attribute 指针，代码随示例复杂度反复膨胀。
 * Mesh 把"一份几何 + 一张贴图集"的 GPU 状态封装成一个结构体：
 * 构造时一次性上传顶点/索引并记录 attribute 布局，绘制时只需要绑定 VAO、
 * 绑定纹理、按索引数发出一次 glDrawElements。
 * 本示例用 Mesh 绘制一个带漫反射/镜面贴图的立方体和一个光源小立方体，
 * 光照着色器与 02_lighting 章节一致，验证封装没有丢失任何渲染能力。
 */

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace {

constexpr int window_width{800};
constexpr int window_height{600};
constexpr const char* window_title{"OpenGL Lab - Mesh"};

#if !defined(OPENGL_LAB_ASSET_ROOT)
#define OPENGL_LAB_ASSET_ROOT "."
#endif

const glm::vec3 light_position{1.2F, 1.5F, 1.2F};
const glm::vec3 view_position{1.5F, 1.2F, 2.5F};

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
 *
 * @details
 * 与交错 float 数组相比，结构体让"一个顶点包含哪些数据"直接体现在类型里，
 * 上传 GPU 时按成员偏移取地址即可。
 */
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

/**
 * @brief Mesh：一份几何与材质纹理的 GPU 封装。
 *
 * @details
 * vao/vbo/ebo 记录 OpenGL 对象句柄，index_count 是绘制所需的索引数量，
 * diffuse_map/specular_map/shininess 是材质部分——几何与材质一起封装，
 * 一个 Mesh 就是一个可以独立绘制的东西。
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
 * @brief 生成纹理文件路径。
 */
std::string texture_path(const char* file_name) {
    std::filesystem::path path{OPENGL_LAB_ASSET_ROOT};
    path /= "textures";
    path /= file_name;
    return path.generic_string();
}

/**
 * @brief 从项目资源目录加载 2D 纹理。
 */
GLuint create_texture(const char* file_name) {
    const std::string path{texture_path(file_name)};

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
 * @brief 把顶点数组和索引数组上传到 GPU，生成一个 Mesh。
 *
 * @details
 * 之前每个示例都要手写的 VAO/VBO/EBO 样板代码集中到这一个函数里：
 * 交错上传 Vertex 结构体（连续内存，成员偏移即 attribute 偏移），
 * attribute 布局固定为 0 = 位置、1 = 法线、2 = 纹理坐标，
 * 与 02_lighting 章节的立方体布局保持一致。
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

    // OpenGL: attribute 偏移按交错布局计算——位置在结构体开头（偏移 0），
    // 法线偏移 3 个 float，纹理坐标偏移 6 个 float；static_assert 保证布局紧密。
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
 *
 * @details
 * 绑定 diffuse/specular 到纹理单元 0/1（与 setup 阶段 material.diffuse = 0、
 * material.specular = 1 的赋值对应），绑定 VAO 后按索引绘制。
 * shader 不采样纹理时（如光源小立方体）这两个绑定只是多余状态，不产生影响。
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
 * @brief 释放 Mesh 持有的 GPU 对象。
 *
 * @pre 仍在 OpenGL 上下文存活期间调用（先于 glfwTerminate）。
 */
void destroy_mesh(Mesh& mesh) {
    glDeleteBuffers(1, &mesh.element_buffer_object);
    glDeleteBuffers(1, &mesh.vertex_buffer_object);
    glDeleteVertexArrays(1, &mesh.vertex_array_object);
}

}  // namespace

/**
 * @brief mesh 示例入口。
 *
 * @return 正常运行返回 EXIT_SUCCESS；初始化、编译或纹理加载失败返回 EXIT_FAILURE。
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

    const GLuint diffuse_map{create_texture("container_diffuse.ppm")};
    const GLuint specular_map{create_texture("container_specular.ppm")};
    if (diffuse_map == 0U || specular_map == 0U) {
        glDeleteTextures(1, &specular_map);
        glDeleteTextures(1, &diffuse_map);
        glDeleteProgram(light_program);
        glDeleteProgram(object_program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // 与 02_lighting 章节相同的立方体数据（位置/法线/纹理坐标交错，36 个顶点），
    // 这里按 8 个 float 一组打包进 Vertex 结构体，展示数据布局的转换。
    constexpr std::array<float, 288> cube_vertices{
        // 位置坐标             // 法线方向           // 纹理坐标
        -0.5F, -0.5F, -0.5F, 0.0F,  0.0F,  -1.0F, 0.0F,  0.0F,  0.5F,  -0.5F, -0.5F, 0.0F,
        0.0F,  -1.0F, 1.0F,  0.0F,  0.5F,  0.5F,  -0.5F, 0.0F,  0.0F,  -1.0F, 1.0F,  1.0F,
        0.5F,  0.5F,  -0.5F, 0.0F,  0.0F,  -1.0F, 1.0F,  1.0F,  -0.5F, 0.5F,  -0.5F, 0.0F,
        0.0F,  -1.0F, 0.0F,  1.0F,  -0.5F, -0.5F, -0.5F, 0.0F,  0.0F,  -1.0F, 0.0F,  0.0F,

        -0.5F, -0.5F, 0.5F,  0.0F,  0.0F,  1.0F,  0.0F,  0.0F,  0.5F,  -0.5F, 0.5F,  0.0F,
        0.0F,  1.0F,  1.0F,  0.0F,  0.5F,  0.5F,  0.5F,  0.0F,  0.0F,  1.0F,  1.0F,  1.0F,
        0.5F,  0.5F,  0.5F,  0.0F,  0.0F,  1.0F,  1.0F,  1.0F,  -0.5F, 0.5F,  0.5F,  0.0F,
        0.0F,  1.0F,  0.0F,  1.0F,  -0.5F, -0.5F, 0.5F,  0.0F,  0.0F,  1.0F,  0.0F,  0.0F,

        -0.5F, 0.5F,  0.5F,  -1.0F, 0.0F,  0.0F,  1.0F,  0.0F,  -0.5F, 0.5F,  -0.5F, -1.0F,
        0.0F,  0.0F,  1.0F,  1.0F,  -0.5F, -0.5F, -0.5F, -1.0F, 0.0F,  0.0F,  0.0F,  1.0F,
        -0.5F, -0.5F, -0.5F, -1.0F, 0.0F,  0.0F,  0.0F,  1.0F,  -0.5F, -0.5F, 0.5F,  -1.0F,
        0.0F,  0.0F,  0.0F,  0.0F,  -0.5F, 0.5F,  0.5F,  -1.0F, 0.0F,  0.0F,  1.0F,  0.0F,

        0.5F,  0.5F,  0.5F,  1.0F,  0.0F,  0.0F,  1.0F,  0.0F,  0.5F,  0.5F,  -0.5F, 1.0F,
        0.0F,  0.0F,  1.0F,  1.0F,  0.5F,  -0.5F, -0.5F, 1.0F,  0.0F,  0.0F,  0.0F,  1.0F,
        0.5F,  -0.5F, -0.5F, 1.0F,  0.0F,  0.0F,  0.0F,  1.0F,  0.5F,  -0.5F, 0.5F,  1.0F,
        0.0F,  0.0F,  0.0F,  0.0F,  0.5F,  0.5F,  0.5F,  1.0F,  0.0F,  0.0F,  1.0F,  0.0F,

        -0.5F, -0.5F, -0.5F, 0.0F,  -1.0F, 0.0F,  0.0F,  1.0F,  0.5F,  -0.5F, -0.5F, 0.0F,
        -1.0F, 0.0F,  1.0F,  1.0F,  0.5F,  -0.5F, 0.5F,  0.0F,  -1.0F, 0.0F,  1.0F,  0.0F,
        0.5F,  -0.5F, 0.5F,  0.0F,  -1.0F, 0.0F,  0.0F,  0.0F,  -0.5F, -0.5F, 0.5F,  0.0F,
        -1.0F, 0.0F,  0.0F,  0.0F,  -0.5F, -0.5F, -0.5F, 0.0F,  -1.0F, 0.0F,  0.0F,  1.0F,

        -0.5F, 0.5F,  -0.5F, 0.0F,  1.0F,  0.0F,  0.0F,  1.0F,  0.5F,  0.5F,  -0.5F, 0.0F,
        1.0F,  0.0F,  1.0F,  1.0F,  0.5F,  0.5F,  0.5F,  0.0F,  1.0F,  0.0F,  1.0F,  0.0F,
        0.5F,  0.5F,  0.5F,  0.0F,  1.0F,  0.0F,  1.0F,  0.0F,  -0.5F, 0.5F,  0.5F,  0.0F,
        1.0F,  0.0F,  0.0F,  0.0F,  -0.5F, 0.5F,  -0.5F, 0.0F,  1.0F,  0.0F,  0.0F,  1.0F,
    };

    std::vector<Vertex> box_vertices;
    box_vertices.reserve(cube_vertices.size() / 8U);
    for (std::size_t offset{0}; offset < cube_vertices.size(); offset += 8U) {
        box_vertices.push_back(
            Vertex{glm::vec3{cube_vertices[offset], cube_vertices[offset + 1U],
                             cube_vertices[offset + 2U]},
                   glm::vec3{cube_vertices[offset + 3U], cube_vertices[offset + 4U],
                             cube_vertices[offset + 5U]},
                   glm::vec2{cube_vertices[offset + 6U], cube_vertices[offset + 7U]}});
    }

    // 索引就是 0..35 的顺序编号：立方体数据本身已按三角形展开，索引绘制保持一致性。
    std::vector<GLuint> box_indices(cube_vertices.size() / 8U, 0U);
    std::iota(box_indices.begin(), box_indices.end(), 0U);

    // Mesh: 贴图箱子与光源小立方体共用同一份几何，各自持有独立的 GPU 对象。
    Mesh box_mesh{make_mesh(box_vertices, box_indices, diffuse_map, specular_map, 32.0F)};
    Mesh light_mesh{make_mesh(box_vertices, box_indices, 0U, 0U, 1.0F)};

    const glm::mat4 view{
        glm::lookAt(view_position, glm::vec3{0.0F, 0.0F, 0.0F}, glm::vec3{0.0F, 1.0F, 0.0F})};
    const glm::mat4 projection{glm::perspective(
        glm::radians(45.0F), static_cast<float>(window_width) / static_cast<float>(window_height),
        0.1F, 100.0F)};

    glUseProgram(object_program);
    glUniform1i(glGetUniformLocation(object_program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(object_program, "material.specular"), 1);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        glClearColor(0.08F, 0.10F, 0.14F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(object_program);
        glUniform1f(glGetUniformLocation(object_program, "material.shininess"), box_mesh.shininess);
        glUniform3fv(glGetUniformLocation(object_program, "view_pos"), 1,
                     glm::value_ptr(view_position));
        glUniform3fv(glGetUniformLocation(object_program, "light.position"), 1,
                     glm::value_ptr(light_position));
        glUniform3f(glGetUniformLocation(object_program, "light.ambient"), 0.10F, 0.10F, 0.10F);
        glUniform3f(glGetUniformLocation(object_program, "light.diffuse"), 0.80F, 0.80F, 0.80F);
        glUniform3f(glGetUniformLocation(object_program, "light.specular"), 1.0F, 1.0F, 1.0F);
        glUniformMatrix4fv(glGetUniformLocation(object_program, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(object_program, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));

        // Camera: 相机固定，箱子自转展示各面贴图与高光。
        glm::mat4 model{1.0F};
        model = glm::rotate(model, glm::radians(static_cast<float>(glfwGetTime()) * 25.0F),
                            glm::vec3{0.7F, 1.0F, 0.4F});
        glUniformMatrix4fv(glGetUniformLocation(object_program, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));

        draw_mesh(box_mesh);

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

        draw_mesh(light_mesh);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // OpenGL: Mesh 的 GPU 对象必须在上下文销毁前释放。
    destroy_mesh(light_mesh);
    destroy_mesh(box_mesh);
    glDeleteTextures(1, &specular_map);
    glDeleteTextures(1, &diffuse_map);
    glDeleteProgram(light_program);
    glDeleteProgram(object_program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
