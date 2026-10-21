/**
 * @file main.cpp
 * @brief 帧缓冲独立示例。
 * @details 使用 OpenGL 3.3 Core；Esc 退出，支持 --frames 与 --capture 有限帧检查。
 */

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

/** @brief 创建 VAO/VBO；顶点布局由调用方在返回后明确配置。 */
GLuint create_vertex_array(gpu_resources& resources, const float* data, GLsizeiptr bytes) {
    GLuint vao{0U};
    GLuint vbo{0U};
    // OpenGL: 创建保存属性格式和缓冲引用的 VAO 名称。
    glGenVertexArrays(1, &vao);
    // OpenGL: 创建缓冲对象名称。
    glGenBuffers(1, &vbo);
    resources.arrays.push_back(vao);
    resources.buffers.push_back(vbo);
    // OpenGL: 后续属性配置记录到此 VAO。
    glBindVertexArray(vao);
    // OpenGL: 选择后续缓冲操作的目标对象。
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // OpenGL: 分配数据存储；nullptr 表示内容尚未初始化。
    glBufferData(GL_ARRAY_BUFFER, bytes, data, GL_STATIC_DRAW);
    // OpenGL: 捕获当前 ARRAY_BUFFER，并指定属性格式、步长和字节偏移。
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    // OpenGL: 使绘制时从缓冲读取该属性。
    glEnableVertexAttribArray(0);
    return vao;
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
/** @brief 从立方体外部观察时，每个三角形均为逆时针环绕。 */
constexpr std::array<float, 108> cube_vertices{
    -0.5F, -0.5F, -0.5F, 0.5F,  0.5F,  -0.5F, 0.5F,  -0.5F, -0.5F, 0.5F,  0.5F, -0.5F, -0.5F, -0.5F,
    -0.5F, -0.5F, 0.5F,  -0.5F, -0.5F, -0.5F, 0.5F,  0.5F,  -0.5F, 0.5F,  0.5F, 0.5F,  0.5F,  0.5F,
    0.5F,  0.5F,  -0.5F, 0.5F,  0.5F,  -0.5F, -0.5F, 0.5F,  -0.5F, 0.5F,  0.5F, -0.5F, 0.5F,  -0.5F,
    -0.5F, -0.5F, -0.5F, -0.5F, -0.5F, -0.5F, -0.5F, -0.5F, 0.5F,  -0.5F, 0.5F, 0.5F,  0.5F,  0.5F,
    0.5F,  0.5F,  -0.5F, -0.5F, 0.5F,  0.5F,  -0.5F, 0.5F,  -0.5F, -0.5F, 0.5F, 0.5F,  0.5F,  0.5F,
    -0.5F, 0.5F,  -0.5F, -0.5F, -0.5F, 0.5F,  -0.5F, -0.5F, 0.5F,  -0.5F, 0.5F, 0.5F,  -0.5F, 0.5F,
    -0.5F, -0.5F, 0.5F,  -0.5F, -0.5F, -0.5F, -0.5F, 0.5F,  -0.5F, 0.5F,  0.5F, 0.5F,  0.5F,  0.5F,
    -0.5F, 0.5F,  0.5F,  0.5F,  -0.5F, 0.5F,  -0.5F, -0.5F, 0.5F,  0.5F,
};
constexpr const char* vertex_shader_source{R"glsl(
#version 330 core
layout (location = 0) in vec3 a_pos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 local_pos;
void main()
{
    local_pos = a_pos;
    gl_Position = projection * view * model * vec4(a_pos, 1.0);
}
)glsl"};
constexpr const char* fragment_shader_source{R"glsl(
#version 330 core
in vec3 local_pos;
out vec4 frag_color;
uniform vec4 object_color;
void main()
{
    float pattern = mod(floor((local_pos.x + 0.5) * 8.0) +
                        floor((local_pos.z + 0.5) * 8.0), 2.0);
    frag_color = vec4(object_color.rgb * mix(0.72, 1.0, pattern), object_color.a);
}
)glsl"};

/** @brief 缓存所有场景 uniform 的位置。 */
struct scene_uniforms {
    GLint model{-1};
    GLint view{-1};
    GLint projection{-1};
    GLint color{-1};
    [[nodiscard]] bool valid() const {
        return model >= 0 && view >= 0 && projection >= 0 && color >= 0;
    }
};

/** @brief 只查询本例实际使用的四个场景参数。 */
scene_uniforms find_scene_uniforms(GLuint program) {
    return {uniform_location(program, "model"), uniform_location(program, "view"),
            uniform_location(program, "projection"), uniform_location(program, "object_color")};
}

/** @brief 上传物体变换与颜色；调用方已绑定场景 program 和立方体 VAO。 */
void draw_cube(const scene_uniforms& uniforms, const glm::vec3& position, const glm::vec3& scale,
               const glm::vec4& color) {
    const glm::mat4 model{glm::scale(glm::translate(glm::mat4{1.0F}, position), scale)};
    // OpenGL: GLM 按列存储矩阵，GL_FALSE 表示不转置。
    glUniformMatrix4fv(uniforms.model, 1, GL_FALSE, glm::value_ptr(model));
    // OpenGL: 从连续四分量数据设置颜色。
    glUniform4fv(uniforms.color, 1, glm::value_ptr(color));
    // OpenGL: 按照当前 VAO 和程序解释顶点并提交图元。
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

/** @brief 地板与两个立方体用于观察遮挡、后处理和多重采样。 */
void draw_scene(const scene_uniforms& uniforms) {
    draw_cube(uniforms, {0.0F, -0.85F, 0.0F}, {6.0F, 0.1F, 6.0F}, {0.25F, 0.3F, 0.4F, 1.0F});
    draw_cube(uniforms, {-0.8F, -0.25F, 0.0F}, {1.0F, 1.0F, 1.0F}, {1.0F, 0.48F, 0.2F, 1.0F});
    draw_cube(uniforms, {0.8F, -0.25F, -1.3F}, {1.0F, 1.0F, 1.0F}, {0.15F, 0.7F, 0.85F, 1.0F});
}
constexpr const char* screen_vertex_source{R"glsl(
#version 330 core
layout (location = 0) in vec3 a_pos;
out vec2 tex_coord;
void main()
{
    tex_coord = a_pos.xy * 0.5 + 0.5;
    gl_Position = vec4(a_pos, 1.0);
}
)glsl"};
constexpr const char* screen_fragment_source{R"glsl(
#version 330 core
in vec2 tex_coord;
out vec4 frag_color;
uniform sampler2D screen_texture;
uniform int effect;
void main()
{
    vec3 color = texture(screen_texture, tex_coord).rgb;
    if (effect == 1) color = vec3(1.0) - color;
    if (effect == 2) color = vec3(dot(color, vec3(0.2126, 0.7152, 0.0722)));
    if (effect == 3) {
        vec2 step_size = 1.0 / vec2(textureSize(screen_texture, 0));
        color = 9.0 * color;
        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                if (x != 0 || y != 0) color -= texture(screen_texture, tex_coord + vec2(x, y) * step_size).rgb;
            }
        }
    }
    frag_color = vec4(color, 1.0);
}
)glsl"};

/** @brief 离屏附件尺寸随窗口变化；FBO 仅组织附件，存储由纹理/RBO 提供。 */
struct render_target {
    GLuint framebuffer{0U};
    GLuint color{0U};
    GLuint depth_stencil{0U};
    int width{0};
    int height{0};
};

/** @brief 创建单采样颜色纹理与深度/模板渲染缓冲，并检查完整性。 */
bool resize_target(gpu_resources& resources, render_target& target, int width, int height) {
    if (target.framebuffer == 0U) {
        // OpenGL: 创建用于组织离屏附件的帧缓冲对象。
        glGenFramebuffers(1, &target.framebuffer);
        // OpenGL: 创建纹理对象名称。
        glGenTextures(1, &target.color);
        // OpenGL: 创建不需要普通纹理采样的附件存储对象。
        glGenRenderbuffers(1, &target.depth_stencil);
        resources.framebuffers.push_back(target.framebuffer);
        resources.textures.push_back(target.color);
        resources.renderbuffers.push_back(target.depth_stencil);
    }
    // OpenGL: GL_FRAMEBUFFER 同时设置读目标与写目标。
    glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
    if (target.width != width || target.height != height) {
        // OpenGL: 绑定纹理到当前活动纹理单元的指定目标。
        glBindTexture(GL_TEXTURE_2D, target.color);
        // OpenGL: 定义纹理图像存储；空指针仅分配空间。
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     nullptr);
        // OpenGL: 配置纹理过滤与环绕状态。
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // OpenGL: 将纹理的一个图像层接到当前 FBO。
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.color,
                               0);
        // OpenGL: 选择后续存储分配使用的 RBO。
        glBindRenderbuffer(GL_RENDERBUFFER, target.depth_stencil);
        // OpenGL: 分配单采样深度/模板附件。
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        // OpenGL: 将渲染缓冲接到 FBO 的指定附件点。
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  target.depth_stencil);
        target.width = width;
        target.height = height;
    }
    // OpenGL: 渲染前检查附件格式、采样数和读写目标等完整性条件。
    const GLenum status{glCheckFramebufferStatus(GL_FRAMEBUFFER)};
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer incomplete: " << status << '\n';
        return false;
    }
    return true;
}

/** @brief 覆盖整个默认帧缓冲的两个三角形。 */
constexpr std::array<float, 18> screen_vertices{-1.0F, -1.0F, 0.0F, 1.0F,  -1.0F, 0.0F,
                                                1.0F,  1.0F,  0.0F, -1.0F, -1.0F, 0.0F,
                                                1.0F,  1.0F,  0.0F, -1.0F, 1.0F,  0.0F};

/** @brief 在当前有效上下文内创建资源并运行渲染循环。 */
int run(GLFWwindow* window, const run_options& options) {
    gpu_resources resources{};

    const GLuint program{create_program(resources, vertex_shader_source, fragment_shader_source)};
    if (program == 0U) {
        return EXIT_FAILURE;
    }
    const scene_uniforms uniforms{find_scene_uniforms(program)};
    if (!uniforms.valid()) {
        return EXIT_FAILURE;
    }
    const GLuint vao{create_vertex_array(resources, cube_vertices.data(), sizeof(cube_vertices))};
    // OpenGL: 后续 uniform 更新和绘制使用此程序。
    glUseProgram(program);
    // OpenGL: 启用当前示例所需的固定功能状态。
    glEnable(GL_DEPTH_TEST);

    const GLuint screen_program{
        create_program(resources, screen_vertex_source, screen_fragment_source)};
    if (screen_program == 0U) {
        return EXIT_FAILURE;
    }
    const GLint screen_texture_uniform{uniform_location(screen_program, "screen_texture")};
    const GLint effect_uniform{uniform_location(screen_program, "effect")};
    if (screen_texture_uniform < 0 || effect_uniform < 0) {
        return EXIT_FAILURE;
    }
    const GLuint screen_vao{
        create_vertex_array(resources, screen_vertices.data(), sizeof(screen_vertices))};
    render_target target{};
    int effect{options.initial_mode};
    bool previous_space{false};

    int frame{0};
    // GLFW: 取得单调计时器秒数，用于计算帧间隔。
    double last_frame_time{glfwGetTime()};
    // GLFW: 检查窗口关闭请求，不在这里销毁上下文。
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        const double now{glfwGetTime()};
        process_input(window, static_cast<float>(now - last_frame_time));
        last_frame_time = now;
        int width{0};
        int height{0};
        // GLFW: 取得实际附件像素尺寸，包括高 DPI 比例。
        glfwGetFramebufferSize(window, &width, &height);
        if (width <= 0 || height <= 0) {
            // GLFW: 最小化时等待事件，避免零尺寸分配和忙循环。
            glfwWaitEvents();
            continue;
        }
        glViewport(0, 0, width, height);
        // OpenGL: 设置之后清除颜色缓冲使用的值。
        glClearColor(0.045F, 0.06F, 0.09F, 1.0F);

        glUseProgram(program);
        glBindVertexArray(vao);
        const glm::mat4 view{
            glm::lookAt(camera_position, camera_position + camera_front, camera_up)};
        const glm::mat4 projection{
            glm::perspective(glm::radians(field_of_view),
                             static_cast<float>(width) / static_cast<float>(height), 0.1F, 100.0F)};
        glUniformMatrix4fv(uniforms.view, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uniforms.projection, 1, GL_FALSE, glm::value_ptr(projection));

        if (key_pressed(window, GLFW_KEY_SPACE, previous_space)) {
            effect = (effect + 1) % 4;
        }
        if (!resize_target(resources, target, width, height)) {
            return EXIT_FAILURE;
        }
        // OpenGL: 采样解绑离屏颜色纹理，明确结束上一帧的屏幕读取阶段。
        glBindTexture(GL_TEXTURE_2D, 0);
        glEnable(GL_DEPTH_TEST);
        // OpenGL: 清除已绑定绘制帧缓冲；清除受相关写掩码限制。
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw_scene(uniforms);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        // OpenGL: 关闭指定状态；多阶段绘制结束后恢复下一阶段的前置条件。
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(screen_program);
        // OpenGL: 写入标量整型或布尔 uniform；sampler 接收纹理单元索引。
        glUniform1i(screen_texture_uniform, 0);
        glUniform1i(effect_uniform, effect);
        // OpenGL: 选择纹理单元；sampler uniform 中填写相应的零起始索引。
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, target.color);
        glBindVertexArray(screen_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
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
                options.initial_mode < 0 || options.initial_mode >= 4) {
                std::cerr << "--mode must be in [0, " << 4 - 1 << "]\n";
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
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    if (options.frame_limit > 0) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    // GLFW: 创建窗口及其 OpenGL 上下文。
    GLFWwindow* window{glfwCreateWindow(window_width, window_height, "OpenGL Lab - 05_framebuffers",
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
