/**
 * @file main.cpp
 * @brief 使用顶点属性在 Shader 之间传递颜色。
 *
 * @details
 * 本示例继续绘制一个三角形，但顶点数据从“只有位置”扩展为“位置 + 颜色”。
 * 顶点着色器读取两个 vertex attribute，把颜色通过 out 变量传给片段着色器。
 * GPU 会在光栅化阶段自动插值三个顶点颜色，因此三角形表面会出现平滑渐变。
 */

#include <array>
#include <cstdlib>
#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace {

constexpr int window_width{800};
constexpr int window_height{600};
constexpr const char* window_title{"OpenGL Lab - Shaders"};

constexpr const char* vertex_shader_source{R"glsl(
#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_color;

out vec3 vertex_color;

void main()
{
    gl_Position = vec4(a_pos, 1.0);
    vertex_color = a_color;
}
)glsl"};

constexpr const char* fragment_shader_source{R"glsl(
#version 330 core
in vec3 vertex_color;
out vec4 frag_color;

void main()
{
    frag_color = vec4(vertex_color, 1.0);
}
)glsl"};

/**
 * @brief 同步 OpenGL 视口和 GLFW 帧缓冲尺寸。
 *
 * @param width 新帧缓冲宽度，单位为物理像素。
 * @param height 新帧缓冲高度，单位为物理像素。
 */
void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    // OpenGL: 窗口尺寸变化时同步视口，避免颜色三角形被裁剪或拉伸。
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
 *
 * @pre 已经初始化 GLAD，并且当前线程拥有有效 OpenGL 上下文。
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

}  // namespace

/**
 * @brief shaders 示例入口。
 *
 * @return 正常运行返回 EXIT_SUCCESS；初始化、编译或链接失败返回 EXIT_FAILURE。
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

    constexpr std::array<float, 18> vertices{
        // 位置坐标            // 顶点颜色
        -0.5F, -0.5F, 0.0F, 1.0F, 0.15F, 0.15F,
        0.5F, -0.5F, 0.0F, 0.15F, 0.90F, 0.35F,
        0.0F, 0.5F, 0.0F, 0.20F, 0.45F, 1.0F,
    };

    GLuint vertex_array_object{0};
    GLuint vertex_buffer_object{0};

    glGenVertexArrays(1, &vertex_array_object);
    glGenBuffers(1, &vertex_buffer_object);

    glBindVertexArray(vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(),
        GL_STATIC_DRAW);

    constexpr GLsizei vertex_stride{6 * static_cast<GLsizei>(sizeof(float))};
    constexpr auto color_offset{3 * sizeof(float)};

    // OpenGL: attribute 0 解释顶点位置，layout(location = 0) 与这里的 index 必须一致。
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_stride, nullptr);
    glEnableVertexAttribArray(0);

    // OpenGL: attribute 1 解释顶点颜色，偏移 3 个 float 后读取同一份 VBO 中的数据。
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, vertex_stride, reinterpret_cast<const void*>(color_offset));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        glClearColor(0.08F, 0.10F, 0.14F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vertex_array_object);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vertex_buffer_object);
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteProgram(shader_program);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
