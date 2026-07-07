/**
 * @file main.cpp
 * @brief 使用 GLM 矩阵对带纹理矩形进行平移、旋转和缩放。
 *
 * @details
 * 本示例延续纹理章节的矩形，但在顶点着色器中加入 `uniform mat4 transform`。
 * CPU 侧每帧用 GLM 生成变换矩阵，并通过 glUniformMatrix4fv 上传到 GPU。
 * 顶点着色器把矩阵乘到顶点位置上，由此让同一份顶点数据产生动态旋转效果。
 */

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

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
constexpr const char* window_title{"OpenGL Lab - Transformations"};

#if !defined(OPENGL_LAB_ASSET_ROOT)
#define OPENGL_LAB_ASSET_ROOT "."
#endif

constexpr const char* vertex_shader_source{R"glsl(
#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_coord;

out vec2 tex_coord;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(a_pos, 1.0);
    tex_coord = a_tex_coord;
}
)glsl"};

constexpr const char* fragment_shader_source{R"glsl(
#version 330 core
in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D texture1;

void main()
{
    frag_color = texture(texture1, tex_coord);
}
)glsl"};

struct stbi_image_deleter {
    void operator()(stbi_uc* data) const noexcept {
        stbi_image_free(data);
    }
};

using stbi_image_ptr = std::unique_ptr<stbi_uc, stbi_image_deleter>;

/**
 * @brief 同步 OpenGL 视口和窗口帧缓冲尺寸。
 *
 * @param width GLFW 报告的新帧缓冲宽度，单位为像素。
 * @param height GLFW 报告的新帧缓冲高度，单位为像素。
 */
void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    // OpenGL: 视口控制最终写入默认帧缓冲的区域，变换矩阵不负责窗口像素映射。
    glViewport(0, 0, width, height);
}

/**
 * @brief 处理当前帧的退出输入。
 *
 * @param window 需要读取键盘状态的 GLFW 窗口。
 */
void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

/**
 * @brief 编译一个 GLSL shader object。
 *
 * @param shader_type shader 阶段枚举。
 * @param source GLSL 源码字符串。
 * @return 编译成功返回 shader handle；失败返回 0。
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
 * @brief 链接本示例使用的 Shader Program。
 *
 * @return 链接成功返回 program handle；失败返回 0。
 */
GLuint create_shader_program() {
    const GLuint vertex_shader{compile_shader(GL_VERTEX_SHADER, vertex_shader_source)};
    if (vertex_shader == 0U) {
        return 0U;
    }

    const GLuint fragment_shader{compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source)};
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
    glGetProgramInfoLog(
        shader_program, static_cast<GLsizei>(info_log.size()), nullptr, info_log.data());
    std::cerr << "Failed to link shader program:\n" << info_log.data() << '\n';
    glDeleteProgram(shader_program);
    return 0U;
}

/**
 * @brief 返回纹理资源的绝对路径。
 */
std::string texture_path() {
    std::filesystem::path path{OPENGL_LAB_ASSET_ROOT};
    path /= "textures";
    path /= "checker.ppm";
    return path.generic_string();
}

/**
 * @brief 加载棋盘纹理并创建 OpenGL 2D texture object。
 *
 * @return 成功时返回 texture handle；失败时返回 0。
 */
GLuint create_texture() {
    const std::string path{texture_path()};

    int width{0};
    int height{0};
    int channel_count{0};

    // stb_image: 翻转图片 Y 轴，让纹理坐标 (0, 0) 对应 OpenGL 习惯中的左下角。
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

    // OpenGL: 图片像素从 CPU 内存复制到当前 GL_TEXTURE_2D 对象的 0 级 mipmap。
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        static_cast<GLint>(format),
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        image_data.get());
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

}  // namespace

/**
 * @brief transformations 示例入口。
 *
 * @return 正常运行返回 EXIT_SUCCESS；初始化、shader、纹理或 uniform 查询失败返回 EXIT_FAILURE。
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

    const GLuint shader_program{create_shader_program()};
    if (shader_program == 0U) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    const GLuint texture{create_texture()};
    if (texture == 0U) {
        glDeleteProgram(shader_program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    constexpr std::array<float, 20> vertices{
        // 位置坐标             // 纹理坐标
        0.5F, 0.5F, 0.0F, 1.0F, 1.0F,
        0.5F, -0.5F, 0.0F, 1.0F, 0.0F,
        -0.5F, -0.5F, 0.0F, 0.0F, 0.0F,
        -0.5F, 0.5F, 0.0F, 0.0F, 1.0F,
    };

    constexpr std::array<unsigned int, 6> indices{
        0U, 1U, 3U,
        1U, 2U, 3U,
    };

    GLuint vertex_array_object{0};
    GLuint vertex_buffer_object{0};
    GLuint element_buffer_object{0};

    glGenVertexArrays(1, &vertex_array_object);
    glGenBuffers(1, &vertex_buffer_object);
    glGenBuffers(1, &element_buffer_object);

    glBindVertexArray(vertex_array_object);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
        indices.data(),
        GL_STATIC_DRAW);

    constexpr GLsizei vertex_stride{5 * static_cast<GLsizei>(sizeof(float))};
    constexpr auto texture_coord_offset{3 * sizeof(float)};

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_stride, nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        vertex_stride,
        reinterpret_cast<const void*>(texture_coord_offset));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(shader_program);

    const GLint texture_uniform{glGetUniformLocation(shader_program, "texture1")};
    const GLint transform_uniform{glGetUniformLocation(shader_program, "transform")};
    if (texture_uniform == -1 || transform_uniform == -1) {
        std::cerr << "Failed to find required shader uniforms\n";
        glDeleteBuffers(1, &element_buffer_object);
        glDeleteBuffers(1, &vertex_buffer_object);
        glDeleteVertexArrays(1, &vertex_array_object);
        glDeleteTextures(1, &texture);
        glDeleteProgram(shader_program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // OpenGL: sampler uniform 绑定纹理单元编号。这里的 0 对应 GL_TEXTURE0。
    glUniform1i(texture_uniform, 0);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        glClearColor(0.08F, 0.10F, 0.14F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        // GLM/OpenGL: GLM 默认使用列主序矩阵，glm::value_ptr 可以直接交给 OpenGL。
        // 下面写法的实际作用顺序是 scale -> rotate -> translate，因为矩阵乘法从右向左生效。
        glm::mat4 transform{1.0F};
        transform = glm::translate(transform, glm::vec3{0.35F, -0.20F, 0.0F});
        transform = glm::rotate(
            transform, static_cast<float>(glfwGetTime()), glm::vec3{0.0F, 0.0F, 1.0F});
        transform = glm::scale(transform, glm::vec3{0.75F, 0.75F, 1.0F});

        // OpenGL: glUniformMatrix4fv 把 4x4 矩阵写入当前 program 的 transform uniform。
        glUniformMatrix4fv(transform_uniform, 1, GL_FALSE, glm::value_ptr(transform));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glUseProgram(shader_program);
        glBindVertexArray(vertex_array_object);
        glDrawElements(
            GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &element_buffer_object);
    glDeleteBuffers(1, &vertex_buffer_object);
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shader_program);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}