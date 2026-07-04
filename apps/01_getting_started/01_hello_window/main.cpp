/**
 * @file main.cpp
 * @brief OpenGL 3.3 Core Profile 的最小窗口示例。
 *
 * @details
 * 本示例对应 LearnOpenGL 路线中的第一个基础目标：初始化 GLFW，
 * 创建原生窗口和 OpenGL 上下文，通过 GLAD 加载 OpenGL 函数指针，
 * 然后保持一个渲染循环，直到用户关闭窗口或按下 Escape。
 *
 * 代码刻意把状态都保留在当前翻译单元中，方便初学阶段完整观察
 * “初始化 -> 创建上下文 -> 加载函数 -> 渲染循环 -> 释放资源” 的顺序。
 * 后续章节再逐步提取窗口、Shader、纹理、相机等可复用封装。
 */

#include <cstdlib>
#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace {

/**
 * @brief 示例窗口的初始客户区宽度，单位为屏幕像素。
 *
 * @details
 * GLFW 创建窗口时会把这个值当作期望的窗口大小。注意窗口大小和
 * OpenGL 实际绘制的帧缓冲大小不一定完全一致，尤其是在高 DPI 屏幕上。
 * 因此程序还需要在 @ref framebuffer_size_callback 中根据真实帧缓冲
 * 尺寸更新 OpenGL 视口。
 */
constexpr int window_width{800};

/**
 * @brief 示例窗口的初始客户区高度，单位为屏幕像素。
 *
 * @see window_width
 */
constexpr int window_height{600};

/**
 * @brief 显示在系统窗口标题栏中的 UTF-8 标题。
 */
constexpr const char* window_title{"OpenGL Lab - Hello Window"};

/**
 * @brief 让 OpenGL 视口跟随 GLFW 帧缓冲大小变化。
 *
 * @details
 * OpenGL 最终会把标准化设备坐标映射到当前帧缓冲的一块矩形区域中，
 * 这块矩形区域就是“视口”。当窗口被缩放、最小化、恢复，或者移动到
 * DPI 缩放不同的显示器上时，GLFW 会把新的帧缓冲尺寸传给此回调。
 * 如果不更新视口，渲染结果可能被拉伸、裁剪，或只占窗口的一部分。
 *
 * @param width 新的帧缓冲宽度，单位为物理像素。
 * @param height 新的帧缓冲高度，单位为物理像素。
 *
 * @pre 调用此函数的线程上已经有一个当前 OpenGL 上下文。
 * @note 第一个 `GLFWwindow*` 参数在这里没有使用，因为视口更新只依赖
 *       GLFW 回调传入的帧缓冲宽高。
 */
void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    // OpenGL: glViewport 设置“标准化设备坐标 -> 帧缓冲像素”的映射区域。
    // 这里从左下角 (0, 0) 开始，覆盖整个 GLFW 报告的帧缓冲。
    glViewport(0, 0, width, height);
}

/**
 * @brief 处理当前帧需要响应的键盘输入。
 *
 * @details
 * 这个入门示例只保留一个输入行为：按下 Escape 时请求关闭窗口。
 * `glfwSetWindowShouldClose` 不会立刻销毁窗口，而是设置 GLFW 窗口状态；
 * 渲染循环会在下一次检查 `glfwWindowShouldClose` 时自然退出。
 *
 * @param window 需要查询输入状态的 GLFW 窗口，不能为 nullptr。
 *
 * @pre @p window 指向一个仍然存活的 GLFWwindow，它由 glfwCreateWindow 创建。
 */
void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

}  // namespace

/**
 * @brief hello-window 示例的程序入口。
 *
 * @details
 * 初始化顺序很重要：
 * 1. 先初始化 GLFW，之后才能使用大多数 GLFW API。
 * 2. 通过 GLFW window hint 请求 OpenGL 3.3 Core Profile 上下文。
 * 3. 创建系统窗口，并把它附带的 OpenGL 上下文设为当前上下文。
 * 4. 在当前上下文存在后，通过 GLAD 加载 OpenGL 函数入口地址。
 * 5. 进入渲染循环：处理输入、清空颜色缓冲、交换前后缓冲、轮询事件。
 * 6. 退出循环后销毁窗口，并终止 GLFW。
 *
 * @return 初始化、运行和退出都正常时返回 EXIT_SUCCESS；GLFW 初始化、
 *         窗口创建或 GLAD 初始化失败时返回 EXIT_FAILURE。
 *
 * @note 第一个示例还没有创建 OpenGL 对象。后续示例会继续加入顶点缓冲、
 *       Shader、纹理，以及用于管理 OpenGL 资源生命周期的 RAII 封装。
 */
int main() {
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    // GLFW/OpenGL: window hint 会影响“下一次”创建窗口时请求的 OpenGL 上下文。
    // 这里请求 OpenGL 3.3，是 LearnOpenGL 常用的入门版本。
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // OpenGL: Core Profile 会移除旧版固定管线 API，强制使用现代 OpenGL 路线。
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if defined(__APPLE__)
    // OpenGL/macOS: macOS 创建 Core Profile 上下文时要求开启 forward-compatible。
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window =
        glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // OpenGL: 大多数 OpenGL API 都作用于“当前上下文”。
    // 在调用 GLAD 或任何 gl* 函数之前，必须先把窗口的上下文设为当前上下文。
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // GLAD/OpenGL: OpenGL 函数地址由显卡驱动在运行时提供。
    // GLAD 需要借助 glfwGetProcAddress，在当前上下文存在后加载这些函数指针。
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // OpenGL: 初始视口不会由 GLFW 自动设置。这里先覆盖整个初始窗口尺寸；
    // 后续窗口尺寸变化再由 framebuffer_size_callback 负责同步。
    glViewport(0, 0, window_width, window_height);

    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        process_input(window);

        // OpenGL: glClearColor 只是在 OpenGL 状态机中设置“清屏颜色”，
        // 并不会立即绘制；真正写入颜色缓冲的是下面的 glClear。
        glClearColor(0.10F, 0.14F, 0.18F, 1.0F);

        // OpenGL: GL_COLOR_BUFFER_BIT 表示清空颜色缓冲，也就是当前帧要显示
        // 到窗口中的颜色图像。这里会用 glClearColor 设置的颜色填满它。
        glClear(GL_COLOR_BUFFER_BIT);

        // GLFW/OpenGL: 默认使用双缓冲。OpenGL 先画到后台缓冲，swap 后才显示到屏幕，
        // 这样可以避免用户看到半帧内容或闪烁。
        glfwSwapBuffers(window);

        // GLFW: 处理系统窗口事件，并触发尺寸变化、键鼠输入等回调。
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
