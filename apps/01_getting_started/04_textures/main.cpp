/**
 * @file main.cpp
 * @brief 使用 stb_image 加载图片并绘制带纹理的矩形。
 *
 * @details
 * 本示例加入 Element Buffer Object（EBO）、纹理坐标、2D Texture 和 sampler2D。
 * CPU 侧使用 stb_image 读取项目内的 PPM 图片，随后把像素数据上传到 GPU。
 * 片段着色器根据插值后的纹理坐标采样图片颜色，并输出到默认帧缓冲。
 */

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace {

constexpr int window_width{800};
constexpr int window_height{600};
constexpr const char* window_title{"OpenGL Lab - Textures"};

#if !defined(OPENGL_LAB_ASSET_ROOT)
#define OPENGL_LAB_ASSET_ROOT "."
#endif

constexpr const char* vertex_shader_source{R"glsl(
#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_coord;

out vec2 tex_coord;

void main()
{
    gl_Position = vec4(a_pos, 1.0);
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

/**
 * @brief stb_image 返回内存的释放器。
 *
 * @details
 * `stbi_load` 分配的内存必须用 `stbi_image_free` 释放。把它包进 unique_ptr
 * 可以避免在早返回或错误路径中忘记释放图片数据。
 */
struct stbi_image_deleter {
    void operator()(stbi_uc* data) const noexcept {
        stbi_image_free(data);
    }
};

using stbi_image_ptr = std::unique_ptr<stbi_uc, stbi_image_deleter>;

/**
 * @brief 同步 OpenGL 视口和 GLFW 帧缓冲尺寸。
 *
 * @param width 新帧缓冲宽度，单位为物理像素。
 * @param height 新帧缓冲高度，单位为物理像素。
 */
void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    // OpenGL: 纹理坐标仍然是 0..1，窗口变化只需要更新最终绘制区域。
    glViewport(0, 0, width, height);
}

/**
 * @brief 处理退出输入。
 *
 * @param window 需要查询键盘状态的 GLFW 窗口。
 */
void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

/**
 * @brief 编译指定阶段的 GLSL shader。
 *
 * @param shader_type OpenGL shader 阶段枚举。
 * @param source 以 null 结尾的 GLSL 源码。
 *
 * @return 编译成功返回 shader handle；失败返回 0 并输出驱动日志。
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
 * @brief 链接一个包含顶点阶段和片段阶段的 Shader Program。
 *
 * @return 链接成功返回 Program handle；失败返回 0。
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
 * @brief 解析本示例使用的纹理文件路径。
 *
 * @return 指向 `assets/textures/checker.ppm` 的绝对路径字符串。
 */
std::string texture_path() {
    std::filesystem::path path{OPENGL_LAB_ASSET_ROOT};
    path /= "textures";
    path /= "checker.ppm";
    return path.generic_string();
}

/**
 * @brief 加载图片并创建 OpenGL 2D Texture。
 *
 * @return 成功时返回 texture handle；失败时返回 0。
 *
 * @pre GLAD 已初始化，且当前线程有有效 OpenGL 上下文。
 */
GLuint create_texture() {
    const std::string path{texture_path()};

    int width{0};
    int height{0};
    int channel_count{0};

    // stb_image: OpenGL 纹理坐标原点通常按左下角理解，图片文件常按左上角存储。
    // 翻转 Y 轴后，纹理坐标 (0, 0) 会更符合 OpenGL 入门教程的图示。
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

    // OpenGL: 纹理对象的参数和像素数据都写入当前绑定到 GL_TEXTURE_2D 的对象。
    glBindTexture(GL_TEXTURE_2D, texture);

    // OpenGL: wrap 参数决定纹理坐标超出 0..1 时如何取样。
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // OpenGL: filter 参数决定纹理放大/缩小时如何从相邻像素插值。
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // OpenGL: glTexImage2D 把 CPU 内存中的像素上传到 GPU 纹理存储。
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

    // OpenGL: mipmap 是一组逐级缩小的纹理图，远处采样更稳定，也匹配上面的 min filter。
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

}  // namespace

/**
 * @brief textures 示例入口。
 *
 * @return 正常运行返回 EXIT_SUCCESS；窗口、Shader 或纹理创建失败返回 EXIT_FAILURE。
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

    // OpenGL: EBO 保存顶点索引，让两个三角形可以复用矩形的四个顶点。
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

    // OpenGL: attribute 1 是 vec2 纹理坐标，它告诉片段着色器从图片的哪个位置采样。
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
    if (texture_uniform == -1) {
        std::cerr << "Failed to find sampler uniform: texture1\n";
        glDeleteBuffers(1, &element_buffer_object);
        glDeleteBuffers(1, &vertex_buffer_object);
        glDeleteVertexArrays(1, &vertex_array_object);
        glDeleteTextures(1, &texture);
        glDeleteProgram(shader_program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // OpenGL: sampler2D uniform 保存的是纹理单元编号，不是 texture object handle。
    glUniform1i(texture_uniform, 0);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        glClearColor(0.08F, 0.10F, 0.14F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        // OpenGL: 激活 0 号纹理单元，再把本例 texture 绑定给这个单元。
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glUseProgram(shader_program);
        glBindVertexArray(vertex_array_object);

        // OpenGL: 索引绘制会读取当前 VAO 记录的 GL_ELEMENT_ARRAY_BUFFER。
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
