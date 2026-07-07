/**
 * @file main.cpp
 * @brief 材质示例：用 Material 和 Light 结构体控制光照表现。
 *
 * @details
 * 本示例在基础光照上加入材质参数。材质把物体对环境光、漫反射和镜面反射的响应拆开，
 * 光源也拆成 ambient/diffuse/specular 三部分。这样同一个光照模型可以表现塑料、金属、陶瓷等不同表面。
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
constexpr const char* window_title{"OpenGL Lab - Materials"};
constexpr glm::vec3 light_position{1.2F, 1.0F, 2.0F};
constexpr glm::vec3 view_position{0.0F, 0.0F, 5.0F};
constexpr glm::vec3 light_color{1.0F, 1.0F, 1.0F};

constexpr const char* object_vertex_shader_source{R"glsl(
#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

out vec3 frag_pos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    frag_pos = vec3(model * vec4(a_pos, 1.0));
    normal = mat3(transpose(inverse(model))) * a_normal;
    gl_Position = projection * view * vec4(frag_pos, 1.0);
}
)glsl"};

constexpr const char* object_fragment_shader_source{R"glsl(
#version 330 core
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 frag_pos;
in vec3 normal;
out vec4 frag_color;

uniform vec3 view_pos;
uniform Material material;
uniform Light light;

void main()
{
    vec3 ambient = light.ambient * material.ambient;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light.position - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse * diff * material.diffuse;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular;

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

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

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

    constexpr std::array<float, 216> vertices{
        -0.5F,-0.5F,-0.5F,0.0F,0.0F,-1.0F, 0.5F,-0.5F,-0.5F,0.0F,0.0F,-1.0F, 0.5F,0.5F,-0.5F,0.0F,0.0F,-1.0F,
        0.5F,0.5F,-0.5F,0.0F,0.0F,-1.0F, -0.5F,0.5F,-0.5F,0.0F,0.0F,-1.0F, -0.5F,-0.5F,-0.5F,0.0F,0.0F,-1.0F,
        -0.5F,-0.5F,0.5F,0.0F,0.0F,1.0F, 0.5F,-0.5F,0.5F,0.0F,0.0F,1.0F, 0.5F,0.5F,0.5F,0.0F,0.0F,1.0F,
        0.5F,0.5F,0.5F,0.0F,0.0F,1.0F, -0.5F,0.5F,0.5F,0.0F,0.0F,1.0F, -0.5F,-0.5F,0.5F,0.0F,0.0F,1.0F,
        -0.5F,0.5F,0.5F,-1.0F,0.0F,0.0F, -0.5F,0.5F,-0.5F,-1.0F,0.0F,0.0F, -0.5F,-0.5F,-0.5F,-1.0F,0.0F,0.0F,
        -0.5F,-0.5F,-0.5F,-1.0F,0.0F,0.0F, -0.5F,-0.5F,0.5F,-1.0F,0.0F,0.0F, -0.5F,0.5F,0.5F,-1.0F,0.0F,0.0F,
        0.5F,0.5F,0.5F,1.0F,0.0F,0.0F, 0.5F,0.5F,-0.5F,1.0F,0.0F,0.0F, 0.5F,-0.5F,-0.5F,1.0F,0.0F,0.0F,
        0.5F,-0.5F,-0.5F,1.0F,0.0F,0.0F, 0.5F,-0.5F,0.5F,1.0F,0.0F,0.0F, 0.5F,0.5F,0.5F,1.0F,0.0F,0.0F,
        -0.5F,-0.5F,-0.5F,0.0F,-1.0F,0.0F, 0.5F,-0.5F,-0.5F,0.0F,-1.0F,0.0F, 0.5F,-0.5F,0.5F,0.0F,-1.0F,0.0F,
        0.5F,-0.5F,0.5F,0.0F,-1.0F,0.0F, -0.5F,-0.5F,0.5F,0.0F,-1.0F,0.0F, -0.5F,-0.5F,-0.5F,0.0F,-1.0F,0.0F,
        -0.5F,0.5F,-0.5F,0.0F,1.0F,0.0F, 0.5F,0.5F,-0.5F,0.0F,1.0F,0.0F, 0.5F,0.5F,0.5F,0.0F,1.0F,0.0F,
        0.5F,0.5F,0.5F,0.0F,1.0F,0.0F, -0.5F,0.5F,0.5F,0.0F,1.0F,0.0F, -0.5F,0.5F,-0.5F,0.0F,1.0F,0.0F,
    };

    GLuint cube_vao{0};
    GLuint light_vao{0};
    GLuint vbo{0};
    glGenVertexArrays(1, &cube_vao);
    glGenVertexArrays(1, &light_vao);
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(), GL_STATIC_DRAW);

    constexpr GLsizei stride{6 * static_cast<GLsizei>(sizeof(float))};
    constexpr auto normal_offset{3 * sizeof(float)};
    glBindVertexArray(cube_vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(normal_offset));
    glEnableVertexAttribArray(1);

    glBindVertexArray(light_vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);

    const glm::mat4 view{glm::lookAt(view_position, glm::vec3{0.0F}, glm::vec3{0.0F, 1.0F, 0.0F})};
    const glm::mat4 projection{glm::perspective(glm::radians(45.0F), static_cast<float>(window_width) / static_cast<float>(window_height), 0.1F, 100.0F)};

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);
        glClearColor(0.05F, 0.07F, 0.10F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(object_program);
        // OpenGL/GLSL: 结构体 uniform 用 "material.diffuse" 这种字段路径逐个设置。
        glUniform3f(glGetUniformLocation(object_program, "material.ambient"), 1.0F, 0.50F, 0.31F);
        glUniform3f(glGetUniformLocation(object_program, "material.diffuse"), 1.0F, 0.50F, 0.31F);
        glUniform3f(glGetUniformLocation(object_program, "material.specular"), 0.50F, 0.50F, 0.50F);
        glUniform1f(glGetUniformLocation(object_program, "material.shininess"), 32.0F);
        glUniform3fv(glGetUniformLocation(object_program, "light.position"), 1, glm::value_ptr(light_position));
        glUniform3f(glGetUniformLocation(object_program, "light.ambient"), 0.20F, 0.20F, 0.20F);
        glUniform3f(glGetUniformLocation(object_program, "light.diffuse"), 0.50F, 0.50F, 0.50F);
        glUniform3f(glGetUniformLocation(object_program, "light.specular"), 1.0F, 1.0F, 1.0F);
        glUniform3fv(glGetUniformLocation(object_program, "view_pos"), 1, glm::value_ptr(view_position));
        glUniformMatrix4fv(glGetUniformLocation(object_program, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4{1.0F}));
        glUniformMatrix4fv(glGetUniformLocation(object_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(object_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glUseProgram(light_program);
        glm::mat4 light_model{1.0F};
        light_model = glm::translate(light_model, light_position);
        light_model = glm::scale(light_model, glm::vec3{0.2F});
        glUniform3fv(glGetUniformLocation(light_program, "light_color"), 1, glm::value_ptr(light_color));
        glUniformMatrix4fv(glGetUniformLocation(light_program, "model"), 1, GL_FALSE, glm::value_ptr(light_model));
        glUniformMatrix4fv(glGetUniformLocation(light_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(light_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(light_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &light_vao);
    glDeleteVertexArrays(1, &cube_vao);
    glDeleteProgram(light_program);
    glDeleteProgram(object_program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}