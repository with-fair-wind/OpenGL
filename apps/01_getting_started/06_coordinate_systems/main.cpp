/**
 * @file main.cpp
 * @brief 使用 model/view/projection 矩阵绘制多个带纹理立方体。
 *
 * @details
 * 本示例从平面矩形进入 3D 坐标系统：模型矩阵把物体放到世界中，
 * 观察矩阵表示相机所在的位置和朝向，投影矩阵把 3D 视锥体投影到裁剪空间。
 * 由于立方体存在前后遮挡，本章也首次启用深度测试。
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
constexpr const char* window_title{"OpenGL Lab - Coordinate Systems"};

#if !defined(OPENGL_LAB_ASSET_ROOT)
#define OPENGL_LAB_ASSET_ROOT "."
#endif

constexpr const char* vertex_shader_source{R"glsl(
#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_coord;

out vec2 tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(a_pos, 1.0);
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

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    // OpenGL: 投影矩阵控制透视效果，glViewport 控制裁剪空间到窗口像素的映射。
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

std::string texture_path() {
    std::filesystem::path path{OPENGL_LAB_ASSET_ROOT};
    path /= "textures";
    path /= "checker.ppm";
    return path.generic_string();
}

GLuint create_texture() {
    const std::string path{texture_path()};

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
 * @brief coordinate-systems 示例入口。
 *
 * @return 正常运行返回 EXIT_SUCCESS；资源创建失败返回 EXIT_FAILURE。
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

    // OpenGL: 深度测试会用深度缓冲记录离相机最近的片段，避免后面的面覆盖前面的面。
    glEnable(GL_DEPTH_TEST);

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

    constexpr std::array<float, 180> vertices{
        -0.5F, -0.5F, -0.5F, 0.0F, 0.0F,
        0.5F, -0.5F, -0.5F, 1.0F, 0.0F,
        0.5F, 0.5F, -0.5F, 1.0F, 1.0F,
        0.5F, 0.5F, -0.5F, 1.0F, 1.0F,
        -0.5F, 0.5F, -0.5F, 0.0F, 1.0F,
        -0.5F, -0.5F, -0.5F, 0.0F, 0.0F,

        -0.5F, -0.5F, 0.5F, 0.0F, 0.0F,
        0.5F, -0.5F, 0.5F, 1.0F, 0.0F,
        0.5F, 0.5F, 0.5F, 1.0F, 1.0F,
        0.5F, 0.5F, 0.5F, 1.0F, 1.0F,
        -0.5F, 0.5F, 0.5F, 0.0F, 1.0F,
        -0.5F, -0.5F, 0.5F, 0.0F, 0.0F,

        -0.5F, 0.5F, 0.5F, 1.0F, 0.0F,
        -0.5F, 0.5F, -0.5F, 1.0F, 1.0F,
        -0.5F, -0.5F, -0.5F, 0.0F, 1.0F,
        -0.5F, -0.5F, -0.5F, 0.0F, 1.0F,
        -0.5F, -0.5F, 0.5F, 0.0F, 0.0F,
        -0.5F, 0.5F, 0.5F, 1.0F, 0.0F,

        0.5F, 0.5F, 0.5F, 1.0F, 0.0F,
        0.5F, 0.5F, -0.5F, 1.0F, 1.0F,
        0.5F, -0.5F, -0.5F, 0.0F, 1.0F,
        0.5F, -0.5F, -0.5F, 0.0F, 1.0F,
        0.5F, -0.5F, 0.5F, 0.0F, 0.0F,
        0.5F, 0.5F, 0.5F, 1.0F, 0.0F,

        -0.5F, -0.5F, -0.5F, 0.0F, 1.0F,
        0.5F, -0.5F, -0.5F, 1.0F, 1.0F,
        0.5F, -0.5F, 0.5F, 1.0F, 0.0F,
        0.5F, -0.5F, 0.5F, 1.0F, 0.0F,
        -0.5F, -0.5F, 0.5F, 0.0F, 0.0F,
        -0.5F, -0.5F, -0.5F, 0.0F, 1.0F,

        -0.5F, 0.5F, -0.5F, 0.0F, 1.0F,
        0.5F, 0.5F, -0.5F, 1.0F, 1.0F,
        0.5F, 0.5F, 0.5F, 1.0F, 0.0F,
        0.5F, 0.5F, 0.5F, 1.0F, 0.0F,
        -0.5F, 0.5F, 0.5F, 0.0F, 0.0F,
        -0.5F, 0.5F, -0.5F, 0.0F, 1.0F,
    };

    constexpr std::array<glm::vec3, 10> cube_positions{
        glm::vec3{0.0F, 0.0F, 0.0F},
        glm::vec3{2.0F, 5.0F, -15.0F},
        glm::vec3{-1.5F, -2.2F, -2.5F},
        glm::vec3{-3.8F, -2.0F, -12.3F},
        glm::vec3{2.4F, -0.4F, -3.5F},
        glm::vec3{-1.7F, 3.0F, -7.5F},
        glm::vec3{1.3F, -2.0F, -2.5F},
        glm::vec3{1.5F, 2.0F, -2.5F},
        glm::vec3{1.5F, 0.2F, -1.5F},
        glm::vec3{-1.3F, 1.0F, -1.5F},
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
    const GLint model_uniform{glGetUniformLocation(shader_program, "model")};
    const GLint view_uniform{glGetUniformLocation(shader_program, "view")};
    const GLint projection_uniform{glGetUniformLocation(shader_program, "projection")};
    if (texture_uniform == -1 || model_uniform == -1 || view_uniform == -1 ||
        projection_uniform == -1) {
        std::cerr << "Failed to find required shader uniforms\n";
        glDeleteBuffers(1, &vertex_buffer_object);
        glDeleteVertexArrays(1, &vertex_array_object);
        glDeleteTextures(1, &texture);
        glDeleteProgram(shader_program);
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glUniform1i(texture_uniform, 0);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        glClearColor(0.06F, 0.08F, 0.12F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 view{glm::lookAt(
            glm::vec3{0.0F, 0.0F, 3.0F},
            glm::vec3{0.0F, 0.0F, 0.0F},
            glm::vec3{0.0F, 1.0F, 0.0F})};
        const glm::mat4 projection{glm::perspective(
            glm::radians(45.0F),
            static_cast<float>(window_width) / static_cast<float>(window_height),
            0.1F,
            100.0F)};

        glUniformMatrix4fv(view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUseProgram(shader_program);
        glBindVertexArray(vertex_array_object);

        for (std::size_t index{0}; index < cube_positions.size(); ++index) {
            glm::mat4 model{1.0F};
            model = glm::translate(model, cube_positions[index]);
            const float angle{20.0F * static_cast<float>(index)};
            model = glm::rotate(
                model,
                glm::radians(angle) + static_cast<float>(glfwGetTime()) * 0.25F,
                glm::vec3{1.0F, 0.3F, 0.5F});

            // OpenGL: 每个立方体只改变 model uniform，VAO/VBO 和纹理都可以复用。
            glUniformMatrix4fv(model_uniform, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vertex_buffer_object);
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shader_program);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}