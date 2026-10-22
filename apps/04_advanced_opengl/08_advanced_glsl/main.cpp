/**
 * @file main.cpp
 * @brief 高级 GLSL独立示例。
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
#include <iostream>
#include <memory>
#include <random>
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
layout (std140) uniform Matrices {
    mat4 projection;
    mat4 view;
};
uniform mat4 model;
uniform int demo_mode;
out VertexData {
    vec3 local_pos;
} vertex_out;
void main()
{
    vertex_out.local_pos = a_pos;
    gl_PointSize = 5.0 + float(gl_VertexID % 8);
    gl_Position = projection * view * model * vec4(a_pos, 1.0);
}
)glsl"};
constexpr const char* fragment_shader_source{R"glsl(
#version 330 core
in VertexData {
    vec3 local_pos;
} fragment_in;
uniform int demo_mode;
uniform float frame_width;
out vec4 frag_color;
void main()
{
    vec3 color = vec3(1.0, 0.35, 0.2);
    if (demo_mode == 2) color = gl_FragCoord.x < frame_width * 0.5 ? vec3(1,0,0) : vec3(0,1,0);
    if (demo_mode == 3) color = gl_FrontFacing ? vec3(1,0.5,0.1) : vec3(0.1,0.7,1);
    gl_FragDepth = demo_mode == 4 ? 0.5 : gl_FragCoord.z;
    frag_color = vec4(color * (0.7 + 0.3 * abs(fragment_in.local_pos.y) * 2.0), 1.0);
}
)glsl"};
constexpr const char* second_fragment_source{R"glsl(
#version 330 core
in VertexData {
    vec3 local_pos;
} fragment_in;
uniform int demo_mode;
uniform float frame_width;
out vec4 frag_color;
void main()
{
    vec3 color = vec3(0.15, 0.65, 0.95);
    if (demo_mode == 2) color = gl_FragCoord.x < frame_width * 0.5 ? vec3(1,0,0) : vec3(0,1,0);
    if (demo_mode == 3) color = gl_FrontFacing ? vec3(1,0.5,0.1) : vec3(0.1,0.7,1);
    gl_FragDepth = demo_mode == 4 ? 0.5 : gl_FragCoord.z;
    frag_color = vec4(color * (0.7 + 0.3 * abs(fragment_in.local_pos.y) * 2.0), 1.0);
}
)glsl"};
constexpr const char* third_fragment_source{R"glsl(
#version 330 core
in VertexData {
    vec3 local_pos;
} fragment_in;
uniform int demo_mode;
uniform float frame_width;
out vec4 frag_color;
void main()
{
    vec3 color = vec3(0.2, 0.9, 0.3);
    if (demo_mode == 2) color = gl_FragCoord.x < frame_width * 0.5 ? vec3(1,0,0) : vec3(0,1,0);
    if (demo_mode == 3) color = gl_FrontFacing ? vec3(1,0.5,0.1) : vec3(0.1,0.7,1);
    gl_FragDepth = demo_mode == 4 ? 0.5 : gl_FragCoord.z;
    frag_color = vec4(color * (0.7 + 0.3 * abs(fragment_in.local_pos.y) * 2.0), 1.0);
}
)glsl"};
constexpr const char* fourth_fragment_source{R"glsl(
#version 330 core
in VertexData {
    vec3 local_pos;
} fragment_in;
uniform int demo_mode;
uniform float frame_width;
out vec4 frag_color;
void main()
{
    vec3 color = vec3(0.9, 0.8, 0.2);
    if (demo_mode == 2) color = gl_FragCoord.x < frame_width * 0.5 ? vec3(1,0,0) : vec3(0,1,0);
    if (demo_mode == 3) color = gl_FrontFacing ? vec3(1,0.5,0.1) : vec3(0.1,0.7,1);
    gl_FragDepth = demo_mode == 4 ? 0.5 : gl_FragCoord.z;
    frag_color = vec4(color * (0.7 + 0.3 * abs(fragment_in.local_pos.y) * 2.0), 1.0);
}
)glsl"};

/** @brief 在当前有效上下文内创建资源并运行渲染循环。 */
int run(GLFWwindow* window, const run_options& options) {
    gpu_resources resources{};

    const std::array<GLuint, 4> programs{
        create_program(resources, vertex_shader_source, fragment_shader_source),
        create_program(resources, vertex_shader_source, second_fragment_source),
        create_program(resources, vertex_shader_source, third_fragment_source),
        create_program(resources, vertex_shader_source, fourth_fragment_source)};
    if (std::any_of(programs.begin(), programs.end(), [](GLuint p) { return p == 0U; })) {
        return EXIT_FAILURE;
    }
    std::array<GLint, 4> model_uniforms{};
    for (std::size_t index{0U}; index < programs.size(); ++index) {
        // OpenGL: 查询 program 内的活动 uniform 块索引，失败为 GL_INVALID_INDEX。
        const GLuint block{glGetUniformBlockIndex(programs[index], "Matrices")};
        if (block == GL_INVALID_INDEX) {
            std::cerr << "Matrices uniform block missing\n";
            return EXIT_FAILURE;
        }
        // OpenGL: 把 program 内的块索引关联到上下文绑定点。
        glUniformBlockBinding(programs[index], block, 0);
        model_uniforms[index] = uniform_location(programs[index], "model");
        if (model_uniforms[index] < 0) {
            return EXIT_FAILURE;
        }
    }
    GLuint uniform_buffer{0U};
    glGenBuffers(1, &uniform_buffer);
    resources.buffers.push_back(uniform_buffer);
    static_assert(sizeof(glm::mat4) == 64U);
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    // OpenGL: 把 UBO 对象接到绑定点，使多个 program 共享矩阵。
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
    const GLuint vao{create_vertex_array(resources, cube_vertices.data(), sizeof(cube_vertices))};
    // OpenGL: 启用当前示例所需的固定功能状态。
    glEnable(GL_DEPTH_TEST);

    int mode{options.initial_mode};
    bool previous_space{false};
    glEnable(GL_PROGRAM_POINT_SIZE);

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

        if (key_pressed(window, GLFW_KEY_SPACE, previous_space)) {
            mode = (mode + 1) % 5;
        }
        // OpenGL: 清除已绑定绘制帧缓冲；清除受相关写掩码限制。
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        const glm::mat4 projection{
            glm::perspective(glm::radians(field_of_view),
                             static_cast<float>(width) / static_cast<float>(height), 0.1F, 100.0F)};
        const glm::mat4 view{
            glm::lookAt(camera_position, camera_position + camera_front, camera_up)};
        glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
        // OpenGL: 更新已分配存储中的指定字节范围，不能扩大容量。
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
                        glm::value_ptr(view));
        glBindVertexArray(vao);
        for (std::size_t index{0U}; index < programs.size(); ++index) {
            // OpenGL: 后续 uniform 更新和绘制使用此程序。
            glUseProgram(programs[index]);
            const GLint mode_uniform{uniform_location(programs[index], "demo_mode")};
            const GLint width_uniform{uniform_location(programs[index], "frame_width")};
            if (mode_uniform < 0 || width_uniform < 0) {
                return EXIT_FAILURE;
            }
            // OpenGL: 写入标量整型或布尔 uniform；sampler 接收纹理单元索引。
            // OpenGL: 写入浮点 uniform；与当前活动 program 的类型一致。
            glUniform1i(mode_uniform, mode);
            glUniform1f(width_uniform, static_cast<float>(width));
            const glm::mat4 model{
                glm::translate(glm::mat4{1.0F},
                               {index % 2U == 0U ? -0.8F : 0.8F, index < 2U ? 0.7F : -0.7F, 0.0F})};
            // OpenGL: GLM 按列存储矩阵，GL_FALSE 表示不转置。
            glUniformMatrix4fv(model_uniforms[index], 1, GL_FALSE, glm::value_ptr(model));
            // OpenGL: 按照当前 VAO 和程序解释顶点并提交图元。
            glDrawArrays(mode == 1 ? GL_POINTS : GL_TRIANGLES, 0, 36);
        }
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
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    if (options.frame_limit > 0) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    // GLFW: 创建窗口及其 OpenGL 上下文。
    GLFWwindow* window{glfwCreateWindow(window_width, window_height,
                                        "OpenGL Lab - 08_advanced_glsl", nullptr, nullptr)};
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
