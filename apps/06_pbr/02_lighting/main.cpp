/**
 * @file main.cpp
 * @brief PBR 独立示例：02_lighting。
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
#include <stb_image.h>

namespace {

/** @brief 当前渲染线程是否已发生 API 错误；GLAD 调试回调不可把错误悄悄消费掉。 */
bool api_error{false};
#ifdef GLAD_DEBUG
/** @brief 使用原始函数指针查询，避免递归调用 GLAD 的调试包装层。 */
void after_gl_call(const char* name, void*, int, ...) {
    // OpenGL: 原始查询不再触发 GLAD 回调；保存错误标记，避免查询消费后丢失失败状态。
    for (GLenum error{glad_glGetError()}; error != GL_NO_ERROR; error = glad_glGetError()) {
        std::cerr << "OpenGL call failed: " << name << ": " << error << '\n';
        api_error = true;
    }
}
#endif

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
    bool success{!api_error};
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
    return success && !api_error;
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

/** @brief 交错上传五种属性；offsetof 不依赖 GLM 是否插入填充。 */
mesh upload_mesh(gpu_resources& resources, std::span<const vertex> vertices) {
    GLuint vao{0U};
    GLuint vbo{0U};
    // OpenGL: 创建顶点格式状态和存储对象。
    glGenVertexArrays(1, &vao);
    // OpenGL: 在当前上下文中配置或使用此阶段的资源与状态；失败由初始化检查或帧末错误检查报告。
    glGenBuffers(1, &vbo);
    resources.arrays.push_back(vao);
    resources.buffers.push_back(vbo);
    // OpenGL: 绑定将用于后续属性配置或绘制的 VAO。
    glBindVertexArray(vao);
    // OpenGL: 在当前上下文中配置或使用此阶段的资源与状态；失败由初始化检查或帧末错误检查报告。
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
    // OpenGL: 在当前上下文中配置或使用此阶段的资源与状态；失败由初始化检查或帧末错误检查报告。
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
/** @brief 更新当前程序中已缓存的浮点参数。 */
void set_float(const shader_program& program, const char* name, float value) {
    // OpenGL: 上传单个浮点参数，位置已在初始化时校验。
    glUniform1f(program.uniforms.at(name), value);
}

/** @brief 球面使用解析法线和 UV；极点不通过退化 UV 三角形求切线。 */
mesh make_sphere(gpu_resources& resources) {
    constexpr int longitude_count{64};
    constexpr int latitude_count{32};
    std::vector<vertex> vertices{};
    const auto sample = [](int x, int y) {
        const float u{static_cast<float>(x) / static_cast<float>(longitude_count)};
        const float v{static_cast<float>(y) / static_cast<float>(latitude_count)};
        const float phi{2.0F * std::numbers::pi_v<float> * u};
        const float theta{std::numbers::pi_v<float> * v};
        const glm::vec3 position{std::cos(phi) * std::sin(theta), std::cos(theta),
                                 std::sin(phi) * std::sin(theta)};
        return vertex{position, position, {u, v}, {}, {}};
    };
    for (int y{0}; y < latitude_count; ++y) {
        for (int x{0}; x < longitude_count; ++x) {
            const vertex a{sample(x, y)};
            const vertex b{sample(x + 1, y)};
            const vertex c{sample(x + 1, y + 1)};
            const vertex d{sample(x, y + 1)};
            if (y > 0)
                vertices.insert(vertices.end(), {a, b, d});
            if (y + 1 < latitude_count)
                vertices.insert(vertices.end(), {b, c, d});
        }
    }
    return upload_mesh(resources, vertices);
}

/** @brief 五张独立纹理分别编码颜色、法线、金属度、粗糙度与 AO。 */
struct material_maps {
    std::array<GLuint, 5> textures{};
};

/** @brief 仅反照率使用硬件 sRGB 解码；其余四张均为线性数据。 */
material_maps load_material(gpu_resources& resources, const char* name) {
    constexpr std::array<const char*, 5> files{"albedo.png", "normal.png", "metallic.png",
                                               "roughness.png", "ao.png"};
    material_maps result{};
    for (std::size_t index{0U}; index < files.size(); ++index) {
        const auto path{asset_path("textures/pbr") / name / files[index]};
        result.textures[index] = load_texture(resources, path, index == 0U);
    }
    return result;
}

/** @brief 每个 sampler 使用独立纹理单元，避免与 IBL 立方体采样器类型冲突。 */
void bind_material(const material_maps& material) {
    for (std::size_t index{0U}; index < material.textures.size(); ++index) {
        bind_texture(static_cast<int>(index), material.textures[index]);
    }
}

/** @brief 点光源强度、方向和材质参数在绘制第一个物体前初始化。 */
void setup_surface(const shader_program& program, const glm::mat4& view,
                   const glm::mat4& projection) {
    set_mat4(program, "view", view);
    set_mat4(program, "projection", projection);
    set_vec3(program, "camera_pos", camera_position);
    set_vec3(program, "albedo", {0.5F, 0.0F, 0.0F});
    set_float(program, "metallic", 0.0F);
    set_float(program, "roughness", 0.5F);
    set_float(program, "ao", 1.0F);
    set_int(program, "textured", 0);
    set_int(program, "normal_mapping", 1);
    set_int(program, "display_mode", 0);
    set_int(program, "ambient_mode", 1);
    set_int(program, "direct_enabled", 1);
    set_int(program, "emissive", 0);
    constexpr std::array<const char*, 5> samplers{"albedo_map", "normal_map", "metallic_map",
                                                  "roughness_map", "ao_map"};
    for (std::size_t index{0U}; index < samplers.size(); ++index)
        set_int(program, samplers[index], static_cast<int>(index));
    constexpr std::array<glm::vec3, 4> positions{glm::vec3{-10.0F, 10.0F, 10.0F},
                                                 {10.0F, 10.0F, 10.0F},
                                                 {-10.0F, -10.0F, 10.0F},
                                                 {10.0F, -10.0F, 10.0F}};
    for (std::size_t index{0U}; index < positions.size(); ++index) {
        const std::string position{"light_positions[" + std::to_string(index) + "]"};
        const std::string color{"light_colors[" + std::to_string(index) + "]"};
        set_vec3(program, position.c_str(), positions[index]);
        set_vec3(program, color.c_str(), {300.0F, 300.0F, 300.0F});
    }
}

/** @brief 横向覆盖粗糙度两端，纵向由底到顶覆盖非金属至金属。 */
void draw_grid(const shader_program& program, const mesh& sphere) {
    constexpr int count{7};
    for (int row{0}; row < count; ++row) {
        set_float(program, "metallic", static_cast<float>(row) / static_cast<float>(count - 1));
        for (int column{0}; column < count; ++column) {
            set_float(program, "roughness",
                      std::clamp(static_cast<float>(column) / static_cast<float>(count - 1), 0.05F,
                                 1.0F));
            draw_object(program, sphere,
                        model_matrix({static_cast<float>(column - 3) * 2.5F,
                                      static_cast<float>(row - 3) * 2.5F, 0.0F},
                                     {1.0F, 1.0F, 1.0F}));
        }
    }
}

/** @brief 光源标记用自发光分支，同样只在输出前色调映射一次。 */
void draw_lights(const shader_program& program, const mesh& sphere) {
    set_int(program, "textured", 0);
    set_int(program, "emissive", 1);
    set_vec3(program, "albedo", {10.0F, 10.0F, 10.0F});
    for (const float x : {-10.0F, 10.0F}) {
        for (const float y : {-10.0F, 10.0F}) {
            draw_object(program, sphere, model_matrix({x, y, 10.0F}, {0.4F, 0.4F, 0.4F}));
        }
    }
    set_int(program, "emissive", 0);
}
constexpr const char* scene_vertex_source{R"glsl(
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
    vertex_out.position = world.xyz;
    vertex_out.normal = transpose(inverse(mat3(model))) * a_normal;
    vertex_out.tex_coord = a_tex_coord;
    gl_Position = projection * view * world;
}
)glsl"};

constexpr const char* fragment_shader_source{R"glsl(
#version 330 core
in VertexData { vec3 position; vec3 normal; vec2 tex_coord; } vertex_in;
out vec4 frag_color;
uniform vec3 camera_pos;
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;
uniform int textured;
uniform int normal_mapping;
uniform int display_mode;
uniform int ambient_mode;
uniform int direct_enabled;
uniform int emissive;
uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D metallic_map;
uniform sampler2D roughness_map;
uniform sampler2D ao_map;
uniform vec3 light_positions[4];
uniform vec3 light_colors[4];
const float pi = 3.141592653589793;

// GLSL: 参数 r 是感知粗糙度；GGX 的 alpha=r²，公式中的 alpha²=r⁴。
float distribution_ggx(vec3 n, vec3 h, float r) {
    float alpha = r * r;
    float alpha2 = alpha * alpha;
    float nh = max(dot(n, h), 0.0);
    float denominator = nh * nh * (alpha2 - 1.0) + 1.0;
    return alpha2 / max(pi * denominator * denominator, 0.0000001);
}
float geometry_schlick_ggx(float nx, float k) {
    return nx / max(nx * (1.0 - k) + k, 0.000001);
}
float geometry_smith(vec3 n, vec3 v, vec3 l, float r) {
    float k = (r + 1.0) * (r + 1.0) / 8.0;
    return geometry_schlick_ggx(max(dot(n, v), 0.0), k) *
           geometry_schlick_ggx(max(dot(n, l), 0.0), k);
}
vec3 fresnel_schlick(float cos_theta, vec3 f0) {
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}
vec3 fresnel_schlick_roughness(float nv, vec3 f0, float r) {
    return f0 + (max(vec3(1.0 - r), f0) - f0) * pow(clamp(1.0 - nv, 0.0, 1.0), 5.0);
}
vec3 material_normal() {
    vec3 n = normalize(vertex_in.normal);
    if (textured == 0 || normal_mapping == 0) return n;
    vec3 q1 = dFdx(vertex_in.position);
    vec3 q2 = dFdy(vertex_in.position);
    vec2 st1 = dFdx(vertex_in.tex_coord);
    vec2 st2 = dFdy(vertex_in.tex_coord);
    float determinant = st1.x * st2.y - st1.y * st2.x;
    if (abs(determinant) < 0.0000001) return n;
    vec3 tangent = (q1 * st2.y - q2 * st1.y) / determinant;
    vec3 bitangent = (-q1 * st2.x + q2 * st1.x) / determinant;
    tangent -= n * dot(n, tangent);
    if (dot(tangent, tangent) < 0.0000001) return n;
    tangent = normalize(tangent);
    float sign_uv = dot(cross(n, tangent), bitangent) < 0.0 ? -1.0 : 1.0;
    bitangent = sign_uv * cross(n, tangent);
    vec3 sampled = texture(normal_map, vertex_in.tex_coord).xyz * 2.0 - 1.0;
    return normalize(mat3(tangent, bitangent, n) * sampled);
}

void main() {
    vec3 base_color = textured != 0 ? texture(albedo_map, vertex_in.tex_coord).rgb : albedo;
    float metal = textured != 0 ? texture(metallic_map, vertex_in.tex_coord).r : metallic;
    float r = clamp(textured != 0 ? texture(roughness_map, vertex_in.tex_coord).r : roughness, 0.05, 1.0);
    float occlusion = textured != 0 ? texture(ao_map, vertex_in.tex_coord).r : ao;
    vec3 n = material_normal();
    vec3 v = normalize(camera_pos - vertex_in.position);
    vec3 f0 = mix(vec3(0.04), base_color, metal);
    vec3 outgoing = vec3(0.0);
    for (int index = 0; index < 4; ++index) {
        vec3 delta = light_positions[index] - vertex_in.position;
        float distance2 = max(dot(delta, delta), 0.0001);
        vec3 l = delta * inversesqrt(distance2);
        float nl = max(dot(n, l), 0.0);
        float nv = max(dot(n, v), 0.0);
        if (nl <= 0.0 || nv <= 0.0 || dot(v + l, v + l) < 0.000001) continue;
        vec3 h = normalize(v + l);
        vec3 incident = light_colors[index] / distance2;
        vec3 f = fresnel_schlick(max(dot(h, v), 0.0), f0);
        float d = distribution_ggx(n, h, r);
        float g = geometry_smith(n, v, l, r);
        vec3 specular = d * g * f / max(4.0 * nv * nl, 0.0001);
        vec3 kd = (1.0 - f) * (1.0 - metal);
        outgoing += (kd * base_color / pi + specular) * incident * nl;
    }
    if (direct_enabled == 0) outgoing = vec3(0.0);
    vec3 ambient = ambient_mode != 0 ? 0.03 * base_color * occlusion : vec3(0.0);
    vec3 color = emissive != 0 ? albedo : ambient + outgoing;
    if (display_mode == 1) {
        // GLSL: 故意省略色调映射和输出编码，作为错误对照。
        frag_color = vec4(color, 1.0);
    } else {
        color = color / (color + vec3(1.0));
        frag_color = vec4(pow(max(color, vec3(0.0)), vec3(1.0 / 2.2)), 1.0);
    }
}
)glsl"};

/** @brief 本例保留直接光照球阵列以及真实五贴图材质的前后对照。 */
int run(GLFWwindow* window, const run_options& options) {
    gpu_resources resources{};
    const mesh sphere{make_sphere(resources)};
    const shader_program program{make_shader(resources, scene_vertex_source, fragment_shader_source,
                                             {"model",           "view",
                                              "projection",      "camera_pos",
                                              "albedo",          "metallic",
                                              "roughness",       "ao",
                                              "textured",        "normal_mapping",
                                              "display_mode",    "ambient_mode",
                                              "direct_enabled",  "emissive",
                                              "albedo_map",      "normal_map",
                                              "metallic_map",    "roughness_map",
                                              "ao_map",          "light_positions[0]",
                                              "light_colors[0]", "light_positions[1]",
                                              "light_colors[1]", "light_positions[2]",
                                              "light_colors[2]", "light_positions[3]",
                                              "light_colors[3]"})};
    const material_maps rust{load_material(resources, "rusted_iron")};
    if (program.id == 0U || std::ranges::any_of(rust.textures, [](GLuint id) { return id == 0U; }))
        return EXIT_FAILURE;
    point_camera({0.0F, 0.0F, 27.0F}, {0.0F, 0.0F, 0.0F});
    int mode{options.initial_mode};
    bool previous_space{false};
    int frame{0};
    // GLFW: 在当前上下文中配置或使用此阶段的资源与状态；失败由初始化检查或帧末错误检查报告。
    double last_time{glfwGetTime()};
    // OpenGL: 关闭自动编码/剔除等状态，以服从当前渲染阶段。
    glDisable(GL_FRAMEBUFFER_SRGB);
    // OpenGL: 启用当前上下文中的指定功能状态。
    glEnable(GL_DEPTH_TEST);
    // GLFW: 在当前上下文中配置或使用此阶段的资源与状态；失败由初始化检查或帧末错误检查报告。
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        const double now{glfwGetTime()};
        process_input(window, static_cast<float>(now - last_time));
        last_time = now;
        if (key_pressed(window, GLFW_KEY_SPACE, previous_space))
            mode = (mode + 1) % 5;
        int width{0};
        int height{0};
        // GLFW: 在当前上下文中配置或使用此阶段的资源与状态；失败由初始化检查或帧末错误检查报告。
        glfwGetFramebufferSize(window, &width, &height);
        // GLFW: 在当前上下文中配置或使用此阶段的资源与状态；失败由初始化检查或帧末错误检查报告。
        if (width <= 0 || height <= 0) {
            glfwWaitEvents();
            continue;
        }
        const bool textured{mode == 1 || mode == 4};
        const glm::mat4 view{
            glm::lookAt(camera_position, camera_position + camera_front, camera_up)};
        const glm::mat4 projection{
            glm::perspective(glm::radians(field_of_view),
                             static_cast<float>(width) / static_cast<float>(height), 0.1F, 100.0F)};
        glViewport(0, 0, width, height);
        // OpenGL: 在当前上下文中配置或使用此阶段的资源与状态；失败由初始化检查或帧末错误检查报告。
        glClearColor(0.015F, 0.02F, 0.03F, 1.0F);
        // OpenGL: 在当前上下文中配置或使用此阶段的资源与状态；失败由初始化检查或帧末错误检查报告。
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // OpenGL: 在当前上下文中配置或使用此阶段的资源与状态；失败由初始化检查或帧末错误检查报告。
        glUseProgram(program.id);
        setup_surface(program, view, projection);
        set_int(program, "ambient_mode", mode != 3);
        set_int(program, "display_mode", mode == 2 ? 1 : 0);
        set_int(program, "textured", textured ? 1 : 0);
        set_int(program, "normal_mapping", mode != 4);
        bind_material(rust);
        if (textured) {
            draw_object(program, sphere, model_matrix({0.0F, 0.0F, 0.0F}, {6.0F, 6.0F, 6.0F}));
        } else {
            draw_grid(program, sphere);
        }
        draw_lights(program, sphere);
        if (!finish_frame(window, options, ++frame, width, height))
            return EXIT_FAILURE;
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
                options.initial_mode < 0 || options.initial_mode >= 5) {
                std::cerr << "--mode must be in [0, " << 5 - 1 << "]\n";
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
    // GLFW: 请求 sRGB 能力；本章在 shader 最后手动编码，避免重复转换。
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    if (options.frame_limit > 0) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    // GLFW: 创建窗口及其 OpenGL 上下文。
    GLFWwindow* window{glfwCreateWindow(window_width, window_height, "OpenGL Lab - 02_lighting",
                                        nullptr, nullptr)};
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
#ifdef GLAD_DEBUG
    // OpenGL: GLAD 调试版本的回调记录错误到 api_error，使有限帧运行返回失败。
    glad_set_post_callback(after_gl_call);
#endif
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
    // OpenGL: 非调试生成器也检查资源销毁或末次事件回调留下的错误。
    for (GLenum error{glGetError()}; error != GL_NO_ERROR; error = glGetError()) {
        std::cerr << "OpenGL error: " << error << '\n';
        api_error = true;
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return result == EXIT_SUCCESS && !api_error ? EXIT_SUCCESS : EXIT_FAILURE;
}
