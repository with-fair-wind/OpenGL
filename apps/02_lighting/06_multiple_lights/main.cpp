/**
 * @file main.cpp
 * @brief 多光源示例：组合方向光、点光源和聚光灯。
 *
 * @details
 * 真实场景通常不只有一个光源。本示例把光照计算拆成三个 GLSL helper：
 * 方向光负责基础照明，四个点光源负责局部亮斑，聚光灯模拟从摄像机方向照出的手电筒。
 * 多个光源的结果按 RGB 颜色相加，最后输出到片段颜色。
 */

#include <array>
#include <cmath>
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
constexpr const char* window_title{"OpenGL Lab - Multiple Lights"};
constexpr int point_light_count{4};

#if !defined(OPENGL_LAB_ASSET_ROOT)
#define OPENGL_LAB_ASSET_ROOT "."
#endif

const glm::vec3 view_position{0.0F, 0.0F, 5.0F};
const glm::vec3 view_target{0.0F, 0.0F, 0.0F};
const glm::vec3 light_color{1.0F, 1.0F, 1.0F};
const std::array<glm::vec3, point_light_count> point_light_positions{
    glm::vec3{0.7F, 0.2F, 2.0F},
    glm::vec3{2.3F, -3.3F, -4.0F},
    glm::vec3{-4.0F, 2.0F, -12.0F},
    glm::vec3{0.0F, 0.0F, -3.0F},
};

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
#define POINT_LIGHT_COUNT 4

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cut_off;
    float outer_cut_off;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

in vec3 frag_pos;
in vec3 normal;
in vec2 tex_coord;
out vec4 frag_color;

uniform vec3 view_pos;
uniform Material material;
uniform DirLight dir_light;
uniform PointLight point_lights[POINT_LIGHT_COUNT];
uniform SpotLight spot_light;

vec3 calc_dir_light(DirLight light, vec3 norm, vec3 view_dir)
{
    vec3 light_dir = normalize(-light.direction);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 diffuse_sample = vec3(texture(material.diffuse, tex_coord));
    vec3 specular_sample = vec3(texture(material.specular, tex_coord));
    vec3 ambient = light.ambient * diffuse_sample;
    vec3 diffuse = light.diffuse * diff * diffuse_sample;
    vec3 specular = light.specular * spec * specular_sample;
    return ambient + diffuse + specular;
}

vec3 calc_point_light(PointLight light, vec3 norm, vec3 frag_position, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_position);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    float distance = length(light.position - frag_position);
    float attenuation =
        1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    vec3 diffuse_sample = vec3(texture(material.diffuse, tex_coord));
    vec3 specular_sample = vec3(texture(material.specular, tex_coord));
    vec3 ambient = light.ambient * diffuse_sample;
    vec3 diffuse = light.diffuse * diff * diffuse_sample;
    vec3 specular = light.specular * spec * specular_sample;
    return (ambient + diffuse + specular) * attenuation;
}

vec3 calc_spot_light(SpotLight light, vec3 norm, vec3 frag_position, vec3 view_dir)
{
    vec3 light_dir = normalize(light.position - frag_position);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    float distance = length(light.position - frag_position);
    float attenuation =
        1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);

    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cut_off - light.outer_cut_off;
    float intensity = clamp((theta - light.outer_cut_off) / epsilon, 0.0, 1.0);

    vec3 diffuse_sample = vec3(texture(material.diffuse, tex_coord));
    vec3 specular_sample = vec3(texture(material.specular, tex_coord));
    vec3 ambient = light.ambient * diffuse_sample;
    vec3 diffuse = light.diffuse * diff * diffuse_sample;
    vec3 specular = light.specular * spec * specular_sample;
    diffuse *= intensity;
    specular *= intensity;
    return (ambient + diffuse + specular) * attenuation;
}

void main()
{
    vec3 norm = normalize(normal);
    vec3 view_dir = normalize(view_pos - frag_pos);

    vec3 result = calc_dir_light(dir_light, norm, view_dir);
    for (int i = 0; i < POINT_LIGHT_COUNT; ++i) {
        result += calc_point_light(point_lights[i], norm, frag_pos, view_dir);
    }
    result += calc_spot_light(spot_light, norm, frag_pos, view_dir);

    frag_color = vec4(result, 1.0);
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

}  // namespace

/**
 * @brief multiple lights 示例入口。
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
        glDeleteProgram(object_program);
        glDeleteProgram(light_program);
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

    constexpr std::array<float, 288> vertices{
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
        0.5F,  -0.5F, 0.5F,  0.0F,  -1.0F, 0.0F,  1.0F,  0.0F,  -0.5F, -0.5F, 0.5F,  0.0F,
        -1.0F, 0.0F,  0.0F,  0.0F,  -0.5F, -0.5F, -0.5F, 0.0F,  -1.0F, 0.0F,  0.0F,  1.0F,

        -0.5F, 0.5F,  -0.5F, 0.0F,  1.0F,  0.0F,  0.0F,  1.0F,  0.5F,  0.5F,  -0.5F, 0.0F,
        1.0F,  0.0F,  1.0F,  1.0F,  0.5F,  0.5F,  0.5F,  0.0F,  1.0F,  0.0F,  1.0F,  0.0F,
        0.5F,  0.5F,  0.5F,  0.0F,  1.0F,  0.0F,  1.0F,  0.0F,  -0.5F, 0.5F,  0.5F,  0.0F,
        1.0F,  0.0F,  0.0F,  0.0F,  -0.5F, 0.5F,  -0.5F, 0.0F,  1.0F,  0.0F,  0.0F,  1.0F,
    };

    const std::array<glm::vec3, 10> cube_positions{
        glm::vec3{0.0F, 0.0F, 0.0F},    glm::vec3{2.0F, 5.0F, -15.0F},
        glm::vec3{-1.5F, -2.2F, -2.5F}, glm::vec3{-3.8F, -2.0F, -12.3F},
        glm::vec3{2.4F, -0.4F, -3.5F},  glm::vec3{-1.7F, 3.0F, -7.5F},
        glm::vec3{1.3F, -2.0F, -2.5F},  glm::vec3{1.5F, 2.0F, -2.5F},
        glm::vec3{1.5F, 0.2F, -1.5F},   glm::vec3{-1.3F, 1.0F, -1.5F},
    };

    GLuint cube_vao{0};
    GLuint light_vao{0};
    GLuint vbo{0};
    glGenVertexArrays(1, &cube_vao);
    glGenVertexArrays(1, &light_vao);
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);

    constexpr GLsizei stride{8 * static_cast<GLsizei>(sizeof(float))};
    constexpr auto normal_offset{3 * sizeof(float)};
    constexpr auto texture_offset{6 * sizeof(float)};

    glBindVertexArray(cube_vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(normal_offset));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(texture_offset));
    glEnableVertexAttribArray(2);

    glBindVertexArray(light_vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);

    const glm::mat4 view{glm::lookAt(view_position, view_target, glm::vec3{0.0F, 1.0F, 0.0F})};
    const glm::mat4 projection{glm::perspective(
        glm::radians(45.0F), static_cast<float>(window_width) / static_cast<float>(window_height),
        0.1F, 100.0F)};
    const glm::vec3 spot_direction{glm::normalize(view_target - view_position)};

    glUseProgram(object_program);
    glUniform1i(glGetUniformLocation(object_program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(object_program, "material.specular"), 1);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        glClearColor(0.03F, 0.04F, 0.07F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(object_program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse_map);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specular_map);

        glUniform1f(glGetUniformLocation(object_program, "material.shininess"), 32.0F);
        glUniform3fv(glGetUniformLocation(object_program, "view_pos"), 1,
                     glm::value_ptr(view_position));

        // OpenGL/GLSL: 方向光没有位置，direction 表示所有片段看到的入射方向一致。
        glUniform3f(glGetUniformLocation(object_program, "dir_light.direction"), -0.2F, -1.0F,
                    -0.3F);
        glUniform3f(glGetUniformLocation(object_program, "dir_light.ambient"), 0.05F, 0.05F, 0.05F);
        glUniform3f(glGetUniformLocation(object_program, "dir_light.diffuse"), 0.40F, 0.40F, 0.40F);
        glUniform3f(glGetUniformLocation(object_program, "dir_light.specular"), 0.50F, 0.50F,
                    0.50F);

        for (std::size_t index{0}; index < point_light_positions.size(); ++index) {
            const std::string base{"point_lights[" + std::to_string(index) + "]."};
            glUniform3fv(glGetUniformLocation(object_program, (base + "position").c_str()), 1,
                         glm::value_ptr(point_light_positions[index]));
            glUniform3f(glGetUniformLocation(object_program, (base + "ambient").c_str()), 0.05F,
                        0.05F, 0.05F);
            glUniform3f(glGetUniformLocation(object_program, (base + "diffuse").c_str()), 0.80F,
                        0.80F, 0.80F);
            glUniform3f(glGetUniformLocation(object_program, (base + "specular").c_str()), 1.0F,
                        1.0F, 1.0F);
            glUniform1f(glGetUniformLocation(object_program, (base + "constant").c_str()), 1.0F);
            glUniform1f(glGetUniformLocation(object_program, (base + "linear").c_str()), 0.09F);
            glUniform1f(glGetUniformLocation(object_program, (base + "quadratic").c_str()), 0.032F);
        }

        // OpenGL/GLSL: 聚光灯用 cut_off/outer_cut_off 两个余弦值做柔和边缘。
        glUniform3fv(glGetUniformLocation(object_program, "spot_light.position"), 1,
                     glm::value_ptr(view_position));
        glUniform3fv(glGetUniformLocation(object_program, "spot_light.direction"), 1,
                     glm::value_ptr(spot_direction));
        glUniform1f(glGetUniformLocation(object_program, "spot_light.cut_off"),
                    std::cos(glm::radians(12.5F)));
        glUniform1f(glGetUniformLocation(object_program, "spot_light.outer_cut_off"),
                    std::cos(glm::radians(17.5F)));
        glUniform3f(glGetUniformLocation(object_program, "spot_light.ambient"), 0.0F, 0.0F, 0.0F);
        glUniform3f(glGetUniformLocation(object_program, "spot_light.diffuse"), 1.0F, 1.0F, 1.0F);
        glUniform3f(glGetUniformLocation(object_program, "spot_light.specular"), 1.0F, 1.0F, 1.0F);
        glUniform1f(glGetUniformLocation(object_program, "spot_light.constant"), 1.0F);
        glUniform1f(glGetUniformLocation(object_program, "spot_light.linear"), 0.09F);
        glUniform1f(glGetUniformLocation(object_program, "spot_light.quadratic"), 0.032F);

        glUniformMatrix4fv(glGetUniformLocation(object_program, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(object_program, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));

        glBindVertexArray(cube_vao);
        for (std::size_t index{0}; index < cube_positions.size(); ++index) {
            glm::mat4 model{1.0F};
            model = glm::translate(model, cube_positions[index]);
            model = glm::rotate(model, glm::radians(20.0F * static_cast<float>(index + 1U)),
                                glm::vec3{1.0F, 0.3F, 0.5F});
            glUniformMatrix4fv(glGetUniformLocation(object_program, "model"), 1, GL_FALSE,
                               glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glUseProgram(light_program);
        glUniform3fv(glGetUniformLocation(light_program, "light_color"), 1,
                     glm::value_ptr(light_color));
        glUniformMatrix4fv(glGetUniformLocation(light_program, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(light_program, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));
        glBindVertexArray(light_vao);

        for (const glm::vec3& position : point_light_positions) {
            glm::mat4 light_model{1.0F};
            light_model = glm::translate(light_model, position);
            light_model = glm::scale(light_model, glm::vec3{0.2F});
            glUniformMatrix4fv(glGetUniformLocation(light_program, "model"), 1, GL_FALSE,
                               glm::value_ptr(light_model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &light_vao);
    glDeleteVertexArrays(1, &cube_vao);
    glDeleteTextures(1, &specular_map);
    glDeleteTextures(1, &diffuse_map);
    glDeleteProgram(light_program);
    glDeleteProgram(object_program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
