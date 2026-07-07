/**
 * @file main.cpp
 * @brief 光照章节的颜色入门示例。
 *
 * @details
 * 本示例绘制两个立方体：一个代表被照亮的物体，一个代表光源位置。
 * 物体片段着色器把“物体吸收/反射的颜色”和“光源颜色”相乘，展示 OpenGL
 * 光照计算中最基础的颜色组合关系。这里还没有使用法线和光照方向，下一章再加入。
 */

#include <array>
#include <cstdlib>
#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {

constexpr int window_width{800};
constexpr int window_height{600};
constexpr const char* window_title{"OpenGL Lab - Lighting Colors"};

constexpr glm::vec3 light_position{1.2F, 1.0F, 2.0F};
constexpr glm::vec3 object_color{1.0F, 0.50F, 0.31F};
constexpr glm::vec3 light_color{1.0F, 1.0F, 1.0F};

constexpr const char* object_vertex_shader_source{R"glsl(
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

constexpr const char* object_fragment_shader_source{R"glsl(
#version 330 core
out vec4 frag_color;

uniform vec3 object_color;
uniform vec3 light_color;

void main()
{
    frag_color = vec4(object_color * light_color, 1.0);
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

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    // OpenGL: 3D 场景仍然先输出到裁剪空间，最终再由视口映射到窗口像素。
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

/**
 * @brief 编译一个 GLSL shader object。
 *
 * @param shader_type shader 阶段枚举，例如 GL_VERTEX_SHADER。
 * @param source GLSL 源码字符串。
 * @return 编译成功返回 shader handle；失败输出日志并返回 0。
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
 * @brief 链接一个顶点着色器和片段着色器。
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
    glGetProgramInfoLog(
        shader_program, static_cast<GLsizei>(info_log.size()), nullptr, info_log.data());
    std::cerr << "Failed to link shader program:\n" << info_log.data() << '\n';
    glDeleteProgram(shader_program);
    return 0U;
}

}  // namespace

/**
 * @brief colors 示例入口。
 *
 * @return 正常运行返回 EXIT_SUCCESS；初始化或 OpenGL 资源创建失败返回 EXIT_FAILURE。
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

    const GLuint object_program{create_shader_program(
        object_vertex_shader_source, object_fragment_shader_source)};
    const GLuint light_program{create_shader_program(
        light_vertex_shader_source, light_fragment_shader_source)};
    if (object_program == 0U || light_program == 0U) {
        glDeleteProgram(object_program);
        glDeleteProgram(light_program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    constexpr std::array<float, 108> vertices{
        -0.5F, -0.5F, -0.5F, 0.5F, -0.5F, -0.5F, 0.5F, 0.5F, -0.5F,
        0.5F, 0.5F, -0.5F, -0.5F, 0.5F, -0.5F, -0.5F, -0.5F, -0.5F,
        -0.5F, -0.5F, 0.5F, 0.5F, -0.5F, 0.5F, 0.5F, 0.5F, 0.5F,
        0.5F, 0.5F, 0.5F, -0.5F, 0.5F, 0.5F, -0.5F, -0.5F, 0.5F,
        -0.5F, 0.5F, 0.5F, -0.5F, 0.5F, -0.5F, -0.5F, -0.5F, -0.5F,
        -0.5F, -0.5F, -0.5F, -0.5F, -0.5F, 0.5F, -0.5F, 0.5F, 0.5F,
        0.5F, 0.5F, 0.5F, 0.5F, 0.5F, -0.5F, 0.5F, -0.5F, -0.5F,
        0.5F, -0.5F, -0.5F, 0.5F, -0.5F, 0.5F, 0.5F, 0.5F, 0.5F,
        -0.5F, -0.5F, -0.5F, 0.5F, -0.5F, -0.5F, 0.5F, -0.5F, 0.5F,
        0.5F, -0.5F, 0.5F, -0.5F, -0.5F, 0.5F, -0.5F, -0.5F, -0.5F,
        -0.5F, 0.5F, -0.5F, 0.5F, 0.5F, -0.5F, 0.5F, 0.5F, 0.5F,
        0.5F, 0.5F, 0.5F, -0.5F, 0.5F, 0.5F, -0.5F, 0.5F, -0.5F,
    };

    GLuint vertex_array_object{0};
    GLuint vertex_buffer_object{0};
    GLuint light_vertex_array_object{0};

    glGenVertexArrays(1, &vertex_array_object);
    glGenVertexArrays(1, &light_vertex_array_object);
    glGenBuffers(1, &vertex_buffer_object);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
        vertices.data(),
        GL_STATIC_DRAW);

    glBindVertexArray(vertex_array_object);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * static_cast<GLsizei>(sizeof(float)), nullptr);
    glEnableVertexAttribArray(0);

    // OpenGL: 光源小立方体复用同一个 VBO，只是绑定到另一个 VAO，方便独立绘制。
    glBindVertexArray(light_vertex_array_object);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * static_cast<GLsizei>(sizeof(float)), nullptr);
    glEnableVertexAttribArray(0);

    const glm::mat4 view{glm::lookAt(
        glm::vec3{0.0F, 0.0F, 5.0F},
        glm::vec3{0.0F, 0.0F, 0.0F},
        glm::vec3{0.0F, 1.0F, 0.0F})};
    const glm::mat4 projection{glm::perspective(
        glm::radians(45.0F),
        static_cast<float>(window_width) / static_cast<float>(window_height),
        0.1F,
        100.0F)};

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        glClearColor(0.05F, 0.07F, 0.10F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(object_program);
        glUniform3fv(glGetUniformLocation(object_program, "object_color"), 1, glm::value_ptr(object_color));
        glUniform3fv(glGetUniformLocation(object_program, "light_color"), 1, glm::value_ptr(light_color));
        glUniformMatrix4fv(glGetUniformLocation(object_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(
            glGetUniformLocation(object_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        const glm::mat4 object_model{1.0F};
        glUniformMatrix4fv(
            glGetUniformLocation(object_program, "model"), 1, GL_FALSE, glm::value_ptr(object_model));
        glBindVertexArray(vertex_array_object);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glUseProgram(light_program);
        glUniform3fv(glGetUniformLocation(light_program, "light_color"), 1, glm::value_ptr(light_color));
        glUniformMatrix4fv(glGetUniformLocation(light_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(
            glGetUniformLocation(light_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glm::mat4 light_model{1.0F};
        light_model = glm::translate(light_model, light_position);
        light_model = glm::scale(light_model, glm::vec3{0.20F});
        glUniformMatrix4fv(
            glGetUniformLocation(light_program, "model"), 1, GL_FALSE, glm::value_ptr(light_model));
        glBindVertexArray(light_vertex_array_object);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vertex_buffer_object);
    glDeleteVertexArrays(1, &light_vertex_array_object);
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteProgram(light_program);
    glDeleteProgram(object_program);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}