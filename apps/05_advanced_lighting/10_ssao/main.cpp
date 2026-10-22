/**
 * @file main.cpp
 * @brief SSAO独立示例。
 * @details 使用 OpenGL 3.3 Core；Esc 退出，支持 --frames 与 --capture 有限帧检查。
 */

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <numbers>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb_image.h>

namespace {

/** @brief 初始客户区尺寸；实际绘制使用 GLFW 报告的帧缓冲尺寸。 */
constexpr int window_width{800};
constexpr int window_height{600};

/** @brief 可选命令行参数用于有限帧冒烟运行和保存最后一帧。 */
struct run_options {
    int frame_limit{0};
    int initial_mode{0};
    std::string capture_path{};
};

/** @brief 在有效上下文销毁前释放本示例创建的全部 GPU 对象。 */
struct gpu_resources {
    std::vector<GLuint> buffers{};
    std::vector<GLuint> arrays{};
    std::vector<GLuint> textures{};
    std::vector<GLuint> framebuffers{};
    std::vector<GLuint> renderbuffers{};
    std::vector<GLuint> programs{};

    gpu_resources() = default;
    gpu_resources(const gpu_resources&) = delete;
    gpu_resources& operator=(const gpu_resources&) = delete;
    ~gpu_resources() {
        // OpenGL: 删除缓冲对象；资源守卫在上下文仍有效时执行。
        glDeleteBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
        // OpenGL: 删除顶点数组状态对象。
        glDeleteVertexArrays(static_cast<GLsizei>(arrays.size()), arrays.data());
        // OpenGL: 删除纹理存储。
        glDeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());
        // OpenGL: 删除 FBO；附件对象由资源守卫分别删除。
        glDeleteFramebuffers(static_cast<GLsizei>(framebuffers.size()), framebuffers.data());
        // OpenGL: 删除渲染缓冲存储。
        glDeleteRenderbuffers(static_cast<GLsizei>(renderbuffers.size()), renderbuffers.data());
        for (const GLuint program : programs) {
            // OpenGL: 释放不再使用的程序对象。
            glDeleteProgram(program);
        }
    }
};

/** @brief 以物理像素同步视口；零尺寸窗口由主循环暂停绘制。 */
void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    // OpenGL: 视口尺寸使用帧缓冲像素，而非逻辑窗口尺寸。
    glViewport(0, 0, width, height);
}

/** @brief 编译一个阶段，失败时打印驱动日志并释放 shader。 */
GLuint compile_shader(GLenum type, const char* source) {
    // OpenGL: 创建一个着色器阶段对象。
    const GLuint shader{glCreateShader(type)};
    // OpenGL: 提交内嵌 GLSL，字符串长度以结尾空字符确定。
    glShaderSource(shader, 1, &source, nullptr);
    // OpenGL: 由当前上下文的驱动编译 GLSL。
    glCompileShader(shader);
    GLint success{GL_FALSE};
    // OpenGL: 查询编译状态，避免使用失败的阶段。
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) {
        return shader;
    }
    std::array<char, 4096> log{};
    // OpenGL: 读取驱动的编译诊断。
    glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
    std::cerr << "Shader compilation failed:\n" << log.data() << '\n';
    // OpenGL: 释放阶段引用；已附加的阶段由 program 保持到链接结束。
    glDeleteShader(shader);
    return 0U;
}

/** @brief 链接顶点/片段与可选几何阶段；成功后由 resources 持有 program。 */
GLuint create_program(gpu_resources& resources, const char* vertex, const char* fragment,
                      const char* geometry = nullptr) {
    std::array<GLuint, 3> shaders{};
    shaders[0] = compile_shader(GL_VERTEX_SHADER, vertex);
    shaders[1] = compile_shader(GL_FRAGMENT_SHADER, fragment);
    if (geometry != nullptr) {
        shaders[2] = compile_shader(GL_GEOMETRY_SHADER, geometry);
    }
    const bool compiled{shaders[0] != 0U && shaders[1] != 0U &&
                        (geometry == nullptr || shaders[2] != 0U)};
    GLuint program{0U};
    if (compiled) {
        // OpenGL: 创建跨阶段链接的程序对象。
        program = glCreateProgram();
        for (const GLuint shader : shaders) {
            if (shader != 0U) {
                // OpenGL: 将编译后的阶段附加到程序。
                glAttachShader(program, shader);
            }
        }
        // OpenGL: 链接并校验跨阶段输入/输出接口。
        glLinkProgram(program);
    }
    for (const GLuint shader : shaders) {
        if (shader != 0U) {
            glDeleteShader(shader);
        }
    }
    if (!compiled) {
        return 0U;
    }
    GLint success{GL_FALSE};
    // OpenGL: 查询链接状态。
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        std::array<char, 4096> log{};
        // OpenGL: 读取链接失败的驱动日志。
        glGetProgramInfoLog(program, static_cast<GLsizei>(log.size()), nullptr, log.data());
        std::cerr << "Program link failed:\n" << log.data() << '\n';
        glDeleteProgram(program);
        return 0U;
    }
    resources.programs.push_back(program);
    return program;
}

/** @brief 必需 uniform 应在链接后缓存，避免每帧进行字符串查询。 */
GLint uniform_location(GLuint program, const char* name) {
    // OpenGL: 位置仅在当前链接版本中有效，-1 表示未找到活动变量。
    const GLint location{glGetUniformLocation(program, name)};
    if (location < 0) {
        std::cerr << "Required uniform missing: " << name << '\n';
    }
    return location;
}

/** @brief 按键按下边沿触发一次，避免按住时每帧来回切换。 */
bool key_pressed(GLFWwindow* window, int key, bool& previous) {
    // GLFW: 查询 GLFW 缓存的按键状态。
    const bool down{glfwGetKey(window, key) == GLFW_PRESS};
    const bool pressed{down && !previous};
    previous = down;
    return pressed;
}

/** @brief 记录 OpenGL 错误，并按需在交换前读取默认后缓冲保存 PPM。 */
bool finish_frame(GLFWwindow* window, const run_options& options, int frame, int width,
                  int height) {
    bool success{true};
    if (options.frame_limit > 0 && frame == options.frame_limit && !options.capture_path.empty()) {
        std::vector<unsigned char> pixels(static_cast<std::size_t>(width) *
                                          static_cast<std::size_t>(height) * 3U);
        // OpenGL: 紧密打包 RGB 读回行，避免默认四字节对齐填充。
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        // OpenGL: 从默认双缓冲窗口的后缓冲读取待交换图像。
        glReadBuffer(GL_BACK);
        // OpenGL: 读回当前帧用于可重复的视觉检查；常规运行不执行。
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        std::ofstream output{options.capture_path, std::ios::binary};
        output << "P6\n" << width << ' ' << height << "\n255\n";
        const auto row_bytes{static_cast<std::streamsize>(width) * 3};
        for (int row{height - 1}; row >= 0; --row) {
            const auto offset{static_cast<std::size_t>(row) * static_cast<std::size_t>(row_bytes)};
            output.write(reinterpret_cast<const char*>(pixels.data() + offset), row_bytes);
        }
        if (!output) {
            std::cerr << "Capture write failed: " << options.capture_path << '\n';
            success = false;
        }
    }
    // OpenGL: 检查运行时 API 错误，有限帧运行以非零退出码报告失败。
    for (GLenum error{glGetError()}; error != GL_NO_ERROR; error = glGetError()) {
        std::cerr << "OpenGL error: " << error << '\n';
        success = false;
    }
    // GLFW: 将完成的后缓冲交给窗口系统交换。
    glfwSwapBuffers(window);
    // GLFW: 分发窗口和输入事件，从而触发注册的回调。
    glfwPollEvents();
    if (options.frame_limit > 0 && frame >= options.frame_limit) {
        // GLFW: 标记结束渲染循环，资源在循环退出后释放。
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    return success;
}

// Camera: 复用入门章的键鼠相机；每个示例保留自己的状态与回调。
glm::vec3 camera_position{0.0F, 1.2F, 5.0F};
glm::vec3 camera_front{glm::normalize(glm::vec3{0.0F, -0.2F, -1.0F})};
constexpr glm::vec3 camera_up{0.0F, 1.0F, 0.0F};
float yaw{-90.0F};
float pitch{-11.3099F};
float field_of_view{45.0F};
double last_x{0.0};
double last_y{0.0};
bool first_mouse{true};

/** @brief 鼠标位移转换为角度，限制俯仰以避免视线与上向量平行。 */
void mouse_callback(GLFWwindow*, double x, double y) {
    if (first_mouse) {
        last_x = x;
        last_y = y;
        first_mouse = false;
    }
    yaw += static_cast<float>(x - last_x) * 0.1F;
    pitch = std::clamp(pitch + static_cast<float>(last_y - y) * 0.1F, -89.0F, 89.0F);
    last_x = x;
    last_y = y;
    camera_front = glm::normalize(glm::vec3{
        glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)), glm::sin(glm::radians(pitch)),
        glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))});
}

/** @brief 滚轮改变透视 FOV，单位为度。 */
void scroll_callback(GLFWwindow*, double, double offset) {
    field_of_view = std::clamp(field_of_view - static_cast<float>(offset), 1.0F, 75.0F);
}

/** @brief 以秒计的帧间隔缩放移动量；暂停后限制单帧位移。 */
void process_input(GLFWwindow* window, float delta_time) {
    const float speed{3.0F * std::min(delta_time, 0.1F)};
    const glm::vec3 right{glm::normalize(glm::cross(camera_front, camera_up))};
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera_position += speed * camera_front;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera_position -= speed * camera_front;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera_position -= speed * right;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera_position += speed * right;
    }
}
/** @brief 一个 program 的必需参数在初始化时全部检查并缓存。 */
struct shader_program {
    GLuint id{0U};
    std::map<std::string, GLint> uniforms{};
};

/** @brief 缺少活动 uniform 时使初始化失败；GPU 对象仍由资源守卫释放。 */
shader_program make_shader(gpu_resources& resources, const char* vertex, const char* fragment,
                           std::initializer_list<const char*> names,
                           const char* geometry = nullptr) {
    shader_program result{create_program(resources, vertex, fragment, geometry), {}};
    if (result.id == 0U) {
        return result;
    }
    bool valid{true};
    for (const char* name : names) {
        const GLint location{uniform_location(result.id, name)};
        valid = valid && location >= 0;
        result.uniforms.emplace(name, location);
    }
    if (!valid) {
        result.id = 0U;
    }
    return result;
}

/** @brief 更新前要求相应 program 已由 glUseProgram 设为当前程序。 */
void set_int(const shader_program& program, const char* name, int value) {
    // OpenGL: sampler 和布尔参数都通过整型 uniform 更新。
    glUniform1i(program.uniforms.at(name), value);
}
/** @brief 更新当前程序的浮点标量，位置来自初始化时的检查结果。 */
void set_float(const shader_program& program, const char* name, float value) {
    // OpenGL: 上传单个浮点参数。
    glUniform1f(program.uniforms.at(name), value);
}
/** @brief 更新当前程序中的三分量世界/观察空间向量。 */
void set_vec3(const shader_program& program, const char* name, const glm::vec3& value) {
    // OpenGL: GLM 向量通过 value_ptr 提供连续分量。
    glUniform3fv(program.uniforms.at(name), 1, glm::value_ptr(value));
}
/** @brief GLM 矩阵按列存储，因此不要求 OpenGL 转置。 */
void set_mat4(const shader_program& program, const char* name, const glm::mat4& value) {
    // GLM/OpenGL: GL_FALSE 保留矩阵的列主序布局。
    glUniformMatrix4fv(program.uniforms.at(name), 1, GL_FALSE, glm::value_ptr(value));
}

/** @brief 明确位置、法线、UV 与切线基的顶点布局。 */
struct vertex {
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 tex_coord{};
    glm::vec3 tangent{};
    glm::vec3 bitangent{};
};
/** @brief 非拥有型绘制记录；实际 VAO/VBO 生命周期属于 gpu_resources。 */
struct mesh {
    GLuint vao{0U};
    GLsizei count{0};
};

/** @brief 从两条几何边与 UV 差分求解切线基；本例生成的三角形 UV 不退化。 */
void append_triangle(std::vector<vertex>& vertices, const std::array<glm::vec3, 3>& points,
                     const std::array<glm::vec2, 3>& uv) {
    const glm::vec3 edge1{points[1] - points[0]};
    const glm::vec3 edge2{points[2] - points[0]};
    const glm::vec2 delta1{uv[1] - uv[0]};
    const glm::vec2 delta2{uv[2] - uv[0]};
    const float determinant{delta1.x * delta2.y - delta2.x * delta1.y};
    const glm::vec3 tangent{glm::normalize((delta2.y * edge1 - delta1.y * edge2) / determinant)};
    const glm::vec3 bitangent{glm::normalize((-delta2.x * edge1 + delta1.x * edge2) / determinant)};
    const glm::vec3 normal{glm::normalize(glm::cross(edge1, edge2))};
    for (std::size_t index{0U}; index < 3U; ++index) {
        vertices.push_back({points[index], normal, uv[index], tangent, bitangent});
    }
}

/** @brief 交错上传五种属性；offsetof 不依赖 GLM 是否插入填充。 */
mesh upload_mesh(gpu_resources& resources, std::span<const vertex> vertices) {
    GLuint vao{0U};
    GLuint vbo{0U};
    // OpenGL: 创建顶点格式状态和存储对象。
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    resources.arrays.push_back(vao);
    resources.buffers.push_back(vbo);
    // OpenGL: 绑定将用于后续属性配置或绘制的 VAO。
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // OpenGL: 分配缓冲存储并复制输入数据，大小以字节计。
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size_bytes()), vertices.data(),
                 GL_STATIC_DRAW);
    constexpr std::array<std::size_t, 5> offsets{
        offsetof(vertex, position), offsetof(vertex, normal), offsetof(vertex, tex_coord),
        offsetof(vertex, tangent), offsetof(vertex, bitangent)};
    for (GLuint index{0U}; index < offsets.size(); ++index) {
        // OpenGL: 记录当前数组缓冲、属性格式、步长和字节偏移。
        glVertexAttribPointer(index, index == 2U ? 2 : 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                              reinterpret_cast<const void*>(offsets[index]));
        // OpenGL: 使指定属性从 VAO 保存的缓冲布局读取。
        glEnableVertexAttribArray(index);
    }
    return {vao, static_cast<GLsizei>(vertices.size())};
}

/** @brief 朝向 +Z 的平面；既可作为物体，也可在专用屏幕程序中覆盖窗口。 */
mesh make_quad(gpu_resources& resources) {
    std::vector<vertex> vertices{};
    append_triangle(vertices,
                    {glm::vec3{-1.0F, -1.0F, 0.0F}, {1.0F, -1.0F, 0.0F}, {1.0F, 1.0F, 0.0F}},
                    {glm::vec2{0.0F, 0.0F}, {1.0F, 0.0F}, {1.0F, 1.0F}});
    append_triangle(vertices,
                    {glm::vec3{-1.0F, -1.0F, 0.0F}, {1.0F, 1.0F, 0.0F}, {-1.0F, 1.0F, 0.0F}},
                    {glm::vec2{0.0F, 0.0F}, {1.0F, 1.0F}, {0.0F, 1.0F}});
    return upload_mesh(resources, vertices);
}

/** @brief 绘制非索引三角形；当前程序决定如何解释已启用的属性。 */
void draw_mesh(const mesh& geometry) {
    glBindVertexArray(geometry.vao);
    // OpenGL: 按当前 VAO 与程序解释非索引三角形顶点。
    glDrawArrays(GL_TRIANGLES, 0, geometry.count);
}

/** @brief 物体模型矩阵 T R S；旋转参数的单位为弧度。 */
glm::mat4 model_matrix(const glm::vec3& position, const glm::vec3& scale, float angle = 0.0F) {
    return glm::scale(glm::rotate(glm::translate(glm::mat4{1.0F}, position), angle,
                                  glm::normalize(glm::vec3{1.0F, 0.5F, 0.3F})),
                      scale);
}

/** @brief 应用材质前设置当前物体变换。 */
void draw_object(const shader_program& program, const mesh& geometry, const glm::mat4& model) {
    set_mat4(program, "model", model);
    draw_mesh(geometry);
}

#ifndef OPENGL_LAB_ASSET_ROOT
#define OPENGL_LAB_ASSET_ROOT "."
#endif
/** @brief 上传后自动释放 stb_image 解码的 CPU 像素。 */
struct stbi_image_deleter {
    void operator()(unsigned char* pixels) const {
        stbi_image_free(pixels);
    }
};
/** @brief 资源路径基于编译时仓库资源根，允许从任意目录启动。 */
std::filesystem::path asset_path(const char* relative) {
    return std::filesystem::path{OPENGL_LAB_ASSET_ROOT} / relative;
}
/** @brief 颜色纹理按需采用 sRGB 内部格式；法线与深度数据必须保持线性。 */
GLuint load_texture(gpu_resources& resources, const std::filesystem::path& path, bool srgb) {
    int width{0};
    int height{0};
    int channels{0};
    const std::unique_ptr<unsigned char, stbi_image_deleter> pixels{
        stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha)};
    if (!pixels) {
        std::cerr << "Texture load failed: " << path << '\n';
        return 0U;
    }
    GLuint texture{0U};
    // OpenGL: 创建纹理名称，实际存储由后续图像分配建立。
    glGenTextures(1, &texture);
    resources.textures.push_back(texture);
    // OpenGL: 将纹理绑定到当前活动单元的指定目标。
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels.get());
    // OpenGL: 设置当前目标纹理的过滤或环绕参数。
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return texture;
}

/** @brief sampler 中写入零起始纹理单元编号，绑定时使用 GL_TEXTURE0 加编号。 */
void bind_texture(int unit, GLuint texture, GLenum target = GL_TEXTURE_2D) {
    // OpenGL: 激活指定单元之后，目标绑定与其他单元相互独立。
    glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(unit));
    glBindTexture(target, texture);
}

/** @brief 设置可重复的初始观察位置，同时使鼠标回调角度与视线保持一致。 */
void point_camera(const glm::vec3& position, const glm::vec3& target) {
    camera_position = position;
    camera_front = glm::normalize(target - position);
    yaw = glm::degrees(std::atan2(camera_front.z, camera_front.x));
    pitch = glm::degrees(std::asin(camera_front.y));
}
/** @brief 单个离屏渲染阶段的附件；存储随物理像素尺寸一起更新。 */
struct color_target {
    GLuint framebuffer{0U};
    std::vector<GLuint> colors{};
    GLuint depth_stencil{0U};
    int width{0};
    int height{0};
};

/** @brief 创建 MRT/单颜色目标，可选同尺寸的 D24S8；每次使用前检查完整性。 */
bool resize_target(gpu_resources& resources, color_target& target, int width, int height,
                   std::span<const GLint> formats, bool depth) {
    if (target.framebuffer == 0U) {
        // OpenGL: FBO 组织附件，纹理/RBO 才拥有实际存储。
        glGenFramebuffers(1, &target.framebuffer);
        resources.framebuffers.push_back(target.framebuffer);
        target.colors.resize(formats.size());
        glGenTextures(static_cast<GLsizei>(target.colors.size()), target.colors.data());
        resources.textures.insert(resources.textures.end(), target.colors.begin(),
                                  target.colors.end());
        if (depth) {
            // OpenGL: 创建供离屏深度/模板使用的渲染缓冲对象名称。
            glGenRenderbuffers(1, &target.depth_stencil);
            resources.renderbuffers.push_back(target.depth_stencil);
        }
    }
    // OpenGL: GL_FRAMEBUFFER 同时选择读目标与绘制目标。
    glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
    if (target.width != width || target.height != height) {
        for (std::size_t index{0U}; index < formats.size(); ++index) {
            bind_texture(0, target.colors[index]);
            glTexImage2D(GL_TEXTURE_2D, 0, formats[index], width, height, 0,
                         formats[index] == GL_R16F ? GL_RED : GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // OpenGL: location 与 glDrawBuffers 指定的附件槽一一对应。
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index), GL_TEXTURE_2D,
                                   target.colors[index], 0);
        }
        if (depth) {
            // OpenGL: 选择随后分配存储或挂接附件的 RBO。
            glBindRenderbuffer(GL_RENDERBUFFER, target.depth_stencil);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
            // OpenGL: 将 RBO 存储挂接到当前 FBO 的指定附件点。
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                      target.depth_stencil);
        }
        constexpr std::array<GLenum, 3> attachments{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                                    GL_COLOR_ATTACHMENT2};
        // OpenGL: 只启用本目标实际拥有的颜色输出。
        glDrawBuffers(static_cast<GLsizei>(formats.size()), attachments.data());
        target.width = width;
        target.height = height;
    }
    // OpenGL: 绘制前验证附件、读写选择等完整性条件。
    const GLenum status{glCheckFramebufferStatus(GL_FRAMEBUFFER)};
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer incomplete: " << status << '\n';
        return false;
    }
    return true;
}
/** @brief 导入本仓库木箱；预变换烘焙节点层级，CPU aiScene 销毁前上传顶点。 */
std::vector<mesh> load_geometry(gpu_resources& resources) {
    Assimp::Importer importer{};
    const auto path{asset_path("models/crate/crate.obj")};
    const aiScene* scene{importer.ReadFile(path.string(), aiProcess_Triangulate |
                                                              aiProcess_GenNormals |
                                                              aiProcess_PreTransformVertices)};
    if (scene == nullptr || scene->mRootNode == nullptr ||
        (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U) {
        std::cerr << "Model load failed: " << path << ": " << importer.GetErrorString() << '\n';
        return {};
    }
    std::vector<mesh> result{};
    for (unsigned int index{0U}; index < scene->mNumMeshes; ++index) {
        const aiMesh& source{*scene->mMeshes[index]};
        if (!source.HasNormals() || !source.HasTextureCoords(0))
            return {};
        std::vector<vertex> vertices{};
        for (unsigned int face{0U}; face < source.mNumFaces; ++face) {
            const aiFace& triangle{source.mFaces[face]};
            if (triangle.mNumIndices != 3U)
                return {};
            for (unsigned int corner{0U}; corner < 3U; ++corner) {
                const unsigned int v{triangle.mIndices[corner]};
                const auto& p{source.mVertices[v]};
                const auto& n{source.mNormals[v]};
                const auto& uv{source.mTextureCoords[0][v]};
                vertices.push_back({{p.x, p.y, p.z}, {n.x, n.y, n.z}, {uv.x, uv.y}, {}, {}});
            }
        }
        result.push_back(upload_mesh(resources, vertices));
    }
    return result;
}
/** @brief 小型随机方向纹理平铺到窗口；数据是线性向量而非颜色。 */
GLuint make_noise_texture(gpu_resources& resources, std::mt19937& generator) {
    std::uniform_real_distribution<float> random{0.0F, 1.0F};
    std::array<glm::vec3, 16> noise{};
    for (glm::vec3& direction : noise) {
        const float angle{random(generator) * 2.0F * std::numbers::pi_v<float>};
        direction = {std::cos(angle), std::sin(angle), 0.0F};
    }
    static_assert(sizeof(glm::vec3) == 3U * sizeof(float));
    GLuint texture{0U};
    glGenTextures(1, &texture);
    resources.textures.push_back(texture);
    bind_texture(0, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, noise.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return texture;
}
constexpr const char* geometry_vertex_source{R"glsl(
#version 330 core
layout(location=0) in vec3 a_pos;
layout(location=1) in vec3 a_normal;
layout(location=2) in vec2 a_tex_coord;
out VertexData { vec3 position; vec3 normal; vec2 tex_coord; } vertex_out;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    vec4 world = model * vec4(a_pos, 1.0);
    vertex_out.position = (view * world).xyz;
    vertex_out.normal = transpose(inverse(mat3(view * model))) * a_normal;
    vertex_out.tex_coord = a_tex_coord;
    gl_Position = projection * view * world;
}
)glsl"};

constexpr const char* geometry_fragment_source{R"glsl(
#version 330 core
in VertexData { vec3 position; vec3 normal; vec2 tex_coord; } vertex_in;
layout(location=0) out vec4 g_position;
layout(location=1) out vec4 g_normal;
layout(location=2) out vec4 g_albedo_spec;
uniform sampler2D diffuse_texture;
uniform sampler2D specular_texture;
void main() {
    g_position = vec4(vertex_in.position,-vertex_in.position.z);
    g_normal = vec4(normalize(vertex_in.normal),1.0);
    g_albedo_spec = vec4(texture(diffuse_texture,vertex_in.tex_coord).rgb,
                         texture(specular_texture,vertex_in.tex_coord).r);
}
)glsl"};

constexpr const char* screen_vertex_source{R"glsl(
#version 330 core
layout(location=0) in vec3 a_pos;
layout(location=2) in vec2 a_tex_coord;
out vec2 tex_coord;
void main() {
    tex_coord = a_tex_coord;
    gl_Position = vec4(a_pos.xy, 0.0, 1.0);
}
)glsl"};

constexpr const char* ssao_fragment_source{R"glsl(
#version 330 core
in vec2 tex_coord;
out float occlusion_factor;
uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D noise_texture;
uniform vec3 samples[64];
uniform mat4 projection;
uniform float radius;
uniform float bias;
void main() {
    vec4 position_data = texture(g_position,tex_coord);
    if (position_data.a <= 0.0) { occlusion_factor = 1.0; return; }
    vec3 position = position_data.xyz;
    vec3 normal = normalize(texture(g_normal,tex_coord).xyz);
    vec2 noise_scale = vec2(textureSize(g_position,0)) / 4.0;
    vec3 random_vector = texture(noise_texture,tex_coord * noise_scale).xyz;
    vec3 tangent = random_vector - normal * dot(random_vector,normal);
    if (dot(tangent,tangent) < 0.000001)
        tangent = cross(normal,abs(normal.z) < 0.9 ? vec3(0,0,1) : vec3(0,1,0));
    tangent = normalize(tangent);
    mat3 tbn = mat3(tangent,cross(normal,tangent),normal);
    float occluded = 0.0;
    for (int i=0; i<64; ++i) {
        vec3 sample_position = position + tbn * samples[i] * radius;
        vec4 clip = projection * vec4(sample_position,1.0);
        if (clip.w <= 0.0) continue;
        vec2 uv = clip.xy / clip.w * 0.5 + 0.5;
        if (any(lessThan(uv,vec2(0.0))) || any(greaterThan(uv,vec2(1.0)))) continue;
        vec4 sampled = texture(g_position,uv);
        if (sampled.a <= 0.0) continue;
        float sample_depth = sampled.z;
        float range_weight = smoothstep(0.0,1.0,radius / max(abs(position.z-sample_depth),0.0001));
        occluded += (sample_depth >= sample_position.z + bias ? 1.0 : 0.0) * range_weight;
    }
    occlusion_factor = 1.0 - occluded / 64.0;
}
)glsl"};

constexpr const char* blur_fragment_source{R"glsl(
#version 330 core
in vec2 tex_coord;
out float occlusion_factor;
uniform sampler2D image;
void main() {
    vec2 texel_size = 1.0 / vec2(textureSize(image,0));
    float sum = 0.0;
    for (int x=-2; x<2; ++x) for (int y=-2; y<2; ++y)
        sum += texture(image,tex_coord+vec2(x,y) * texel_size).r;
    occlusion_factor = sum / 16.0;
}
)glsl"};

constexpr const char* lighting_fragment_source{R"glsl(
#version 330 core
in vec2 tex_coord;
out vec4 frag_color;
uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo_spec;
uniform sampler2D raw_ao;
uniform sampler2D blurred_ao;
uniform vec3 light_pos;
uniform int mode;
void main() {
    vec4 position_data = texture(g_position,tex_coord);
    if (mode == 2 || mode == 3) {
        float value = mode == 2 ? texture(raw_ao,tex_coord).r : texture(blurred_ao,tex_coord).r;
        frag_color = vec4(vec3(value),1.0);
        return;
    }
    if (position_data.a <= 0.0) { frag_color = vec4(0.015,0.02,0.03,1.0); return; }
    vec3 position = position_data.xyz;
    vec3 normal = normalize(texture(g_normal,tex_coord).xyz);
    if (mode == 4) { frag_color = vec4(vec3(position_data.a/15.0),1.0); return; }
    if (mode == 5) { frag_color = vec4(normal*0.5+0.5,1.0); return; }
    vec4 material = texture(g_albedo_spec,tex_coord);
    float ao = mode == 1 ? 1.0 : texture(blurred_ao,tex_coord).r;
    if (mode == 6) ao = pow(ao,2.0);
    vec3 ambient = 0.35 * material.rgb * ao;
    vec3 delta = light_pos-position;
    vec3 light_dir = normalize(delta);
    vec3 view_dir = normalize(-position);
    float diffuse = max(dot(normal,light_dir),0.0);
    float specular = diffuse > 0.0 ? pow(max(dot(normal,normalize(light_dir+view_dir)),0.0),32.0) : 0.0;
    vec3 direct = (material.rgb * diffuse + vec3(material.a * specular)) * vec3(2.0,2.0,3.0) /
        max(dot(delta,delta),0.1);
    vec3 mapped = vec3(1.0)-exp(-(ambient+direct));
    frag_color = vec4(pow(mapped,vec3(1.0/2.2)),1.0);
}
)glsl"};

/** @brief 在当前上下文创建资源、按阶段绘制，所有错误都使有限帧检查失败。 */
int run(GLFWwindow* window, const run_options& options) {
    gpu_resources resources{};
    const mesh quad{make_quad(resources)};
    const shader_program geometry_program{
        make_shader(resources, geometry_vertex_source, geometry_fragment_source,
                    {"model", "view", "projection", "diffuse_texture", "specular_texture"})};
    const shader_program ssao_program{make_shader(
        resources, screen_vertex_source, ssao_fragment_source,
        {"g_position", "g_normal", "noise_texture", "samples[0]", "projection", "radius", "bias"})};
    const shader_program blur_program{
        make_shader(resources, screen_vertex_source, blur_fragment_source, {"image"})};
    const shader_program lighting_program{make_shader(
        resources, screen_vertex_source, lighting_fragment_source,
        {"g_position", "g_normal", "g_albedo_spec", "raw_ao", "blurred_ao", "light_pos", "mode"})};
    const auto imported{load_geometry(resources)};
    const GLuint diffuse{
        load_texture(resources, asset_path("models/crate/crate_diffuse.ppm"), true)};
    const GLuint specular{
        load_texture(resources, asset_path("models/crate/crate_specular.ppm"), false)};
    if (geometry_program.id == 0U || ssao_program.id == 0U || blur_program.id == 0U ||
        lighting_program.id == 0U || diffuse == 0U || specular == 0U || imported.empty())
        return EXIT_FAILURE;
    std::mt19937 generator{42U};
    std::uniform_real_distribution<float> random{0.0F, 1.0F};
    std::array<glm::vec3, 64> kernel{};
    for (std::size_t index{0U}; index < kernel.size(); ++index) {
        const float z{random(generator)};
        const float angle{random(generator) * 2.0F * std::numbers::pi_v<float>};
        const float radial{std::sqrt(std::max(0.0F, 1.0F - z * z))};
        glm::vec3 sample{radial * std::cos(angle), radial * std::sin(angle), z};
        sample *= random(generator);
        const float fraction{static_cast<float>(index) / static_cast<float>(kernel.size())};
        sample *= std::lerp(0.1F, 1.0F, fraction * fraction);
        kernel[index] = sample;
    }
    const GLuint noise{make_noise_texture(resources, generator)};
    color_target g_buffer{};
    color_target raw_ao{};
    color_target blurred_ao{};
    static_assert(sizeof(glm::vec3) == 3U * sizeof(float));

    point_camera({4.0F, 3.0F, 7.0F}, {0.0F, 1.0F, 0.0F});
    int mode{options.initial_mode};
    bool previous_space{false};
    int frame{0};
    // GLFW: 读取单调计时器的秒数，用于计算帧间隔。
    double last_frame_time{glfwGetTime()};
    // OpenGL: 本章默认在片段程序末尾编码，禁止对中间附件自动重复编码。
    glDisable(GL_FRAMEBUFFER_SRGB);
    // OpenGL: 不透明几何阶段启用深度测试，后处理阶段按需关闭。
    glEnable(GL_DEPTH_TEST);
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        const double now{glfwGetTime()};
        process_input(window, static_cast<float>(now - last_frame_time));
        last_frame_time = now;
        int width{0};
        int height{0};
        // GLFW: 查询真实附件像素尺寸，区别于逻辑客户区大小。
        glfwGetFramebufferSize(window, &width, &height);
        if (width <= 0 || height <= 0) {
            // GLFW: 最小化不分配零尺寸附件，也不消耗有限帧配额。
            glfwWaitEvents();
            continue;
        }
        if (key_pressed(window, GLFW_KEY_SPACE, previous_space)) {
            mode = (mode + 1) % 7;
        }
        const glm::mat4 view{
            glm::lookAt(camera_position, camera_position + camera_front, camera_up)};
        const glm::mat4 projection{
            glm::perspective(glm::radians(field_of_view),
                             static_cast<float>(width) / static_cast<float>(height), 0.1F, 100.0F)};
        glViewport(0, 0, width, height);
        // OpenGL: 设定随后颜色清除使用的各分量数值。
        glClearColor(0.015F, 0.02F, 0.03F, 1.0F);
        constexpr std::array<GLint, 3> formats{GL_RGBA16F, GL_RGBA16F, GL_RGBA8};
        constexpr std::array<GLint, 1> ao_format{GL_R16F};
        if (!resize_target(resources, g_buffer, width, height, formats, true) ||
            !resize_target(resources, raw_ao, width, height, ao_format, false) ||
            !resize_target(resources, blurred_ao, width, height, ao_format, false))
            return EXIT_FAILURE;
        glBindFramebuffer(GL_FRAMEBUFFER, g_buffer.framebuffer);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0F, 0.0F, 0.0F, 0.0F);
        // OpenGL: 清除当前绘制目标的所选缓冲，仍受写掩码约束。
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(geometry_program.id);
        set_mat4(geometry_program, "view", view);
        set_mat4(geometry_program, "projection", projection);
        set_int(geometry_program, "diffuse_texture", 0);
        set_int(geometry_program, "specular_texture", 1);
        bind_texture(0, diffuse);
        bind_texture(1, specular);
        draw_object(
            geometry_program, quad,
            glm::scale(glm::rotate(glm::mat4{1.0F}, glm::radians(-90.0F), {1.0F, 0.0F, 0.0F}),
                       {6.0F, 6.0F, 1.0F}));
        draw_object(geometry_program, quad, model_matrix({0.0F, 3.0F, -3.0F}, {6.0F, 3.0F, 1.0F}));
        draw_object(geometry_program, quad,
                    glm::scale(glm::rotate(glm::translate(glm::mat4{1.0F}, {-3.0F, 3.0F, 0.0F}),
                                           glm::radians(90.0F), {0.0F, 1.0F, 0.0F}),
                               {6.0F, 3.0F, 1.0F}));
        for (const mesh& part : imported) {
            draw_object(geometry_program, part,
                        model_matrix({0.0F, 1.0F, 0.0F}, glm::vec3{2.0F}, 0.2F));
            draw_object(geometry_program, part, model_matrix({2.0F, 0.4F, -1.5F}, glm::vec3{0.8F}));
        }
        glDisable(GL_DEPTH_TEST);
        glBindFramebuffer(GL_FRAMEBUFFER, raw_ao.framebuffer);
        glUseProgram(ssao_program.id);
        set_int(ssao_program, "g_position", 0);
        set_int(ssao_program, "g_normal", 1);
        set_int(ssao_program, "noise_texture", 2);
        set_mat4(ssao_program, "projection", projection);
        set_float(ssao_program, "radius", 0.75F);
        set_float(ssao_program, "bias", 0.025F);
        glUniform3fv(ssao_program.uniforms.at("samples[0]"), 64, glm::value_ptr(kernel[0]));
        bind_texture(0, g_buffer.colors[0]);
        bind_texture(1, g_buffer.colors[1]);
        bind_texture(2, noise);
        draw_mesh(quad);

        glBindFramebuffer(GL_FRAMEBUFFER, blurred_ao.framebuffer);
        glUseProgram(blur_program.id);
        set_int(blur_program, "image", 0);
        bind_texture(0, raw_ao.colors[0]);
        draw_mesh(quad);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(lighting_program.id);
        set_int(lighting_program, "g_position", 0);
        set_int(lighting_program, "g_normal", 1);
        set_int(lighting_program, "g_albedo_spec", 2);
        set_int(lighting_program, "raw_ao", 3);
        set_int(lighting_program, "blurred_ao", 4);
        set_vec3(lighting_program, "light_pos",
                 glm::vec3{view * glm::vec4{2.0F, 4.0F, 2.0F, 1.0F}});
        set_int(lighting_program, "mode", mode);
        bind_texture(0, g_buffer.colors[0]);
        bind_texture(1, g_buffer.colors[1]);
        bind_texture(2, g_buffer.colors[2]);
        bind_texture(3, raw_ao.colors[0]);
        bind_texture(4, blurred_ao.colors[0]);
        draw_mesh(quad);

        ++frame;
        if (!finish_frame(window, options, frame, width, height)) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

}  // namespace

/** @brief 初始化 OpenGL 3.3 Core，运行示例后按资源依赖顺序销毁上下文。 */
int main(int argc, char** argv) {
    run_options options{};
    for (int index{1}; index < argc; ++index) {
        const std::string_view argument{argv[index]};
        if (argument == "--frames" && index + 1 < argc) {
            const std::string_view value{argv[++index]};
            const auto [end, error]{
                std::from_chars(value.data(), value.data() + value.size(), options.frame_limit)};
            if (error != std::errc{} || end != value.data() + value.size() ||
                options.frame_limit <= 0) {
                std::cerr << "--frames requires a positive integer\n";
                return EXIT_FAILURE;
            }
        } else if (argument == "--mode" && index + 1 < argc) {
            const std::string_view value{argv[++index]};
            const auto [end, error]{
                std::from_chars(value.data(), value.data() + value.size(), options.initial_mode)};
            if (error != std::errc{} || end != value.data() + value.size() ||
                options.initial_mode < 0 || options.initial_mode >= 7) {
                std::cerr << "--mode must be in [0, " << 7 - 1 << "]\n";
                return EXIT_FAILURE;
            }
        } else if (argument == "--capture" && index + 1 < argc) {
            options.capture_path = argv[++index];
        } else {
            std::cerr << "Usage: " << argv[0]
                      << " [--frames N] [--mode M] [--capture output.ppm]\n";
            return EXIT_FAILURE;
        }
    }
    if (!options.capture_path.empty() && options.frame_limit == 0) {
        std::cerr << "--capture requires --frames\n";
        return EXIT_FAILURE;
    }
    // GLFW: 初始化窗口系统，失败时不继续创建上下文。
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }
    // GLFW: 设置下一次窗口创建的上下文与附件请求。
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    // GLFW: 请求 sRGB 能力；实际编码能力由第二篇显式查询。
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    if (options.frame_limit > 0) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    // GLFW: 创建窗口及其 OpenGL 上下文。
    GLFWwindow* window{
        glfwCreateWindow(window_width, window_height, "OpenGL Lab - 10_ssao", nullptr, nullptr)};
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window\n";
        // GLFW: 窗口与 GPU 资源释放后终止 GLFW。
        glfwTerminate();
        return EXIT_FAILURE;
    }
    // GLFW: 将上下文绑定到当前线程，先于加载 OpenGL 函数。
    glfwMakeContextCurrent(window);
    // OpenGL: 通过 GLFW 查询当前上下文可用的 OpenGL 函数地址。
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD\n";
        // GLFW: GPU 对象销毁后释放窗口及关联上下文。
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    // GLFW: 注册物理像素尺寸变化通知。
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // GLFW: 注册鼠标移动回调以更新视角。
    glfwSetCursorPosCallback(window, mouse_callback);
    // GLFW: 注册滚轮回调以修改 FOV。
    glfwSetScrollCallback(window, scroll_callback);
    // GLFW: 捕获鼠标以支持连续第一人称视角。
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // GLFW: 请求与显示刷新同步，避免无界空转。
    glfwSwapInterval(1);
    // OpenGL: 读取当前驱动标识用于冒烟日志。
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';
    const int result{run(window, options)};
    glfwDestroyWindow(window);
    glfwTerminate();
    return result;
}
