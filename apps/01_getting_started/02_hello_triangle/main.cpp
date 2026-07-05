/**
 * @file main.cpp
 * @brief 绘制第一个 OpenGL 三角形。
 *
 * @details
 * 本示例在 hello-window 的基础上加入现代 OpenGL 渲染所需的最小对象：
 * 顶点缓冲对象（VBO）、顶点数组对象（VAO）和 Shader Program。
 * 三角形顶点先被上传到 GPU 缓冲区，再由顶点着色器解释为裁剪空间坐标，
 * 最后由片段着色器输出固定颜色。
 */

#include <array>
#include <cstdlib>
#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace {

constexpr int window_width{800};
constexpr int window_height{600};
constexpr const char* window_title{"OpenGL Lab - Hello Triangle"};

constexpr const char* vertex_shader_source{R"glsl(
#version 330 core
layout (location = 0) in vec3 a_pos;

void main()
{
    gl_Position = vec4(a_pos, 1.0);
}
)glsl"};

constexpr const char* fragment_shader_source{R"glsl(
#version 330 core
out vec4 frag_color;

void main()
{
    frag_color = vec4(1.0, 0.45, 0.20, 1.0);
}
)glsl"};

/**
 * @brief 同步 OpenGL 视口和 GLFW 帧缓冲尺寸。
 *
 * @param width 新帧缓冲宽度，单位为物理像素。
 * @param height 新帧缓冲高度，单位为物理像素。
 *
 * @pre 当前线程已经绑定本窗口对应的 OpenGL 上下文。
 */
void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    // OpenGL: 视口决定裁剪空间坐标最终映射到帧缓冲的哪一块像素区域。
    glViewport(0, 0, width, height);
}

/**
 * @brief 处理示例中的每帧输入。
 *
 * @param window 需要查询键盘状态的 GLFW 窗口。
 *
 * @pre @p window 必须是仍然存活的 GLFWwindow。
 */
void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

/**
 * @brief 编译一个 GLSL 着色器对象。
 *
 * @param shader_type OpenGL 着色器阶段，例如 GL_VERTEX_SHADER 或 GL_FRAGMENT_SHADER。
 * @param source GLSL 源码字符串，必须在函数调用期间保持有效。
 *
 * @return 编译成功时返回非 0 的 OpenGL shader handle；失败时输出日志并返回 0。
 *
 * @pre 已经初始化 GLAD，并且当前线程拥有有效 OpenGL 上下文。
 */
GLuint compile_shader(GLenum shader_type, const char* source) {
    // OpenGL: glCreateShader 只创建一个“着色器对象”句柄，还没有源码和机器码。
    const GLuint shader{glCreateShader(shader_type)};

    // OpenGL: glShaderSource 把 CPU 侧字符串交给驱动；真正编译发生在 glCompileShader。
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
 * @brief 创建可用于绘制三角形的 Shader Program。
 *
 * @details
 * OpenGL 需要先分别编译各阶段 shader，再把它们链接成一个 Program。
 * Program 链接成功后，单独的 shader object 可以删除；Program 内部已经保留
 * 执行所需的链接结果。
 *
 * @return 链接成功时返回 Program handle；失败时返回 0。
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

    // OpenGL: Program 是多个 shader stage 链接后的可执行 GPU 程序。
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
 * @brief hello-triangle 示例入口。
 *
 * @return 初始化、资源创建和渲染循环都正常时返回 EXIT_SUCCESS；任一步失败返回 EXIT_FAILURE。
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

    constexpr std::array<float, 9> vertices{
        -0.5F, -0.5F, 0.0F,
        0.5F, -0.5F, 0.0F,
        0.0F, 0.5F, 0.0F,
    };

    GLuint vertex_array_object{0};
    GLuint vertex_buffer_object{0};

    // OpenGL: VAO 记录“如何解释顶点缓冲”的状态，后续绘制只需重新绑定 VAO。
    glGenVertexArrays(1, &vertex_array_object);

    // OpenGL: VBO 是 GPU 侧缓冲区，本例用它保存三角形的三个顶点坐标。
    glGenBuffers(1, &vertex_buffer_object);

    glBindVertexArray(vertex_array_object);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(),
        GL_STATIC_DRAW);

    constexpr GLsizei vertex_stride{3 * static_cast<GLsizei>(sizeof(float))};

    // OpenGL: location = 0 对应顶点着色器里的 a_pos。
    // 每个顶点由 3 个 float 组成，不需要归一化，数据从当前绑定的 GL_ARRAY_BUFFER 读取。
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_stride, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        glClearColor(0.10F, 0.12F, 0.16F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        // OpenGL: glUseProgram 让后续 draw call 使用这个已链接的 GPU 程序。
        glUseProgram(shader_program);
        glBindVertexArray(vertex_array_object);

        // OpenGL: 当前 VAO 中有 3 个顶点，GL_TRIANGLES 表示每 3 个顶点组成一个三角形。
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
