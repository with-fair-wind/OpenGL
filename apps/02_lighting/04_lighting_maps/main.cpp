/**
 * @file main.cpp
 * @brief 光照贴图示例：用 diffuse/specular 纹理替代固定材质颜色。
 *
 * @details
 * 本示例延续材质示例的 Phong 光照模型，但把 `material.diffuse` 和
 * `material.specular` 从常量颜色改成 `sampler2D`。这样物体的每个片段都可以从贴图中取得
 * 不同的漫反射颜色和高光强度，是后续模型材质系统的基础。
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
constexpr const char* window_title{"OpenGL Lab - Lighting Maps"};

#if !defined(OPENGL_LAB_ASSET_ROOT)
#define OPENGL_LAB_ASSET_ROOT "."
#endif

const glm::vec3 light_position{1.2F, 1.0F, 2.0F};
const glm::vec3 view_position{0.0F, 0.0F, 5.0F};
const glm::vec3 light_color{1.0F, 1.0F, 1.0F};

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

struct Light {
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
uniform Light light;

void main()
{
    vec3 diffuse_sample = vec3(texture(material.diffuse, tex_coord));
    vec3 specular_sample = vec3(texture(material.specular, tex_coord));

    vec3 ambient = light.ambient * diffuse_sample;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light.position - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuse_sample;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
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
 * @brief stb_image 返回像素内存的释放器。
 *
 * @details
 * `stbi_load` 分配的图片数据不属于 OpenGL，它只在 CPU 侧临时存在。
 * 上传到 GPU 纹理后即可释放，用 unique_ptr 托管可以让错误路径也不泄漏。
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
    // OpenGL: 视口只影响 NDC 到窗口像素的映射，不会改变物体或纹理本身。
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
 * @param shader_type OpenGL shader 阶段枚举，例如 GL_VERTEX_SHADER。
 * @param source 以 null 结尾的 GLSL 源码。
 *
 * @return 编译成功返回 shader handle；失败返回 0 并输出驱动日志。
 *
 * @pre 当前线程已有有效 OpenGL 上下文，且 GLAD 已经加载函数指针。
 */
GLuint compile_shader(GLenum shader_type, const char* source) {
    const GLuint shader{glCreateShader(shader_type)};
    // OpenGL: shader source 会被复制进 shader object，source 字符串之后无需长期保留。
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
 * @param vertex_source 顶点着色器源码。
 * @param fragment_source 片段着色器源码。
 *
 * @return 链接成功返回 program handle；失败返回 0。
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

    // OpenGL: program 链接后已经持有可执行代码，单独的 shader object 可以删除。
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
 * @brief 生成纹理资源路径。
 *
 * @param file_name `assets/textures` 下的文件名。
 *
 * @return 可传给 stb_image 的路径字符串。
 */
std::string texture_path(const char* file_name) {
    std::filesystem::path path{OPENGL_LAB_ASSET_ROOT};
    path /= "textures";
    path /= file_name;
    return path.generic_string();
}

/**
 * @brief 加载图片并创建 OpenGL 2D Texture。
 *
 * @param file_name `assets/textures` 下的图片文件名。
 *
 * @return 成功时返回 texture handle；失败时返回 0。
 *
 * @pre 当前线程已有有效 OpenGL 上下文，且 GLAD 已经加载函数指针。
 */
GLuint create_texture(const char* file_name) {
    const std::string path{texture_path(file_name)};

    int width{0};
    int height{0};
    int channel_count{0};

    // stb_image: 图片文件通常从左上角开始，OpenGL 入门教程中的纹理坐标通常按左下角理解。
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

    // OpenGL: wrap 规则决定纹理坐标超出 0..1 时是重复、夹紧还是镜像。
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // OpenGL: min filter 使用 mipmap，所以需要在上传后调用 glGenerateMipmap。
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // OpenGL: glTexImage2D 把 CPU 图片数据上传到当前绑定的 GL_TEXTURE_2D 对象。
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), width, height, 0, format,
                 GL_UNSIGNED_BYTE, image_data.get());
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

}  // namespace

/**
 * @brief lighting maps 示例入口。
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
    // OpenGL: 深度测试会丢弃被前面片段挡住的片段，立方体才不会按提交顺序互相覆盖。
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
    // OpenGL: VAO 记录 attribute 格式；这里每个顶点由位置、法线、纹理坐标组成。
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(normal_offset));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<const void*>(texture_offset));
    glEnableVertexAttribArray(2);

    glBindVertexArray(light_vao);
    // OpenGL: 灯光小立方体只需要位置 attribute，但可以复用同一个 VBO。
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(0);

    const glm::mat4 model{1.0F};
    const glm::mat4 view{glm::lookAt(view_position, glm::vec3{0.0F}, glm::vec3{0.0F, 1.0F, 0.0F})};
    const glm::mat4 projection{glm::perspective(
        glm::radians(45.0F), static_cast<float>(window_width) / static_cast<float>(window_height),
        0.1F, 100.0F)};

    glUseProgram(object_program);
    // OpenGL/GLSL: sampler2D 保存纹理单元编号，0/1 对应下面激活的 GL_TEXTURE0/GL_TEXTURE1。
    glUniform1i(glGetUniformLocation(object_program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(object_program, "material.specular"), 1);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        glClearColor(0.05F, 0.07F, 0.10F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(object_program);
        // OpenGL: 先激活纹理单元，再把对应 texture object 绑定到该单元的 GL_TEXTURE_2D。
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse_map);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specular_map);

        glUniform1f(glGetUniformLocation(object_program, "material.shininess"), 32.0F);
        glUniform3fv(glGetUniformLocation(object_program, "light.position"), 1,
                     glm::value_ptr(light_position));
        glUniform3f(glGetUniformLocation(object_program, "light.ambient"), 0.20F, 0.20F, 0.20F);
        glUniform3f(glGetUniformLocation(object_program, "light.diffuse"), 0.50F, 0.50F, 0.50F);
        glUniform3f(glGetUniformLocation(object_program, "light.specular"), 1.0F, 1.0F, 1.0F);
        glUniform3fv(glGetUniformLocation(object_program, "view_pos"), 1,
                     glm::value_ptr(view_position));
        glUniformMatrix4fv(glGetUniformLocation(object_program, "model"), 1, GL_FALSE,
                           glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(object_program, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(object_program, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));

        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glUseProgram(light_program);
        glm::mat4 light_model{1.0F};
        light_model = glm::translate(light_model, light_position);
        light_model = glm::scale(light_model, glm::vec3{0.2F});
        glUniform3fv(glGetUniformLocation(light_program, "light_color"), 1,
                     glm::value_ptr(light_color));
        glUniformMatrix4fv(glGetUniformLocation(light_program, "model"), 1, GL_FALSE,
                           glm::value_ptr(light_model));
        glUniformMatrix4fv(glGetUniformLocation(light_program, "view"), 1, GL_FALSE,
                           glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(light_program, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));
        glBindVertexArray(light_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

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
