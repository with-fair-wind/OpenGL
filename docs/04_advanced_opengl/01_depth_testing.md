# 深度测试

| 项目 | 内容 |
| --- | --- |
| 原文 | [Depth testing](https://learnopengl.com/Advanced-OpenGL/Depth-testing) |
| 作者 | JoeyDeVries |
| 来源 | [LearnOpenGL-CN](https://github.com/with-fair-wind/LearnOpenGL-CN/blob/new-theme/docs/04%20Advanced%20OpenGL/01%20Depth%20testing.md)，按本仓库代码整理修订 |
| 本仓库示例 | [`apps/04_advanced_opengl/01_depth_testing/`](../../apps/04_advanced_opengl/01_depth_testing/) |

**深度测试回答的是“当前片段能否通过已有深度的比较”，深度写入则决定比较通过后是否更新记录。** 前面坐标系章节已经启用了它；本节把开关、比较函数、写掩码和投影深度分开解释。

## 从投影到可见表面

```mermaid
flowchart LR
    A[观察空间位置] --> B[投影到裁剪坐标]
    B --> C[裁剪与透视除法]
    C --> D[NDC 深度映射到窗口深度]
    D --> E[与深度附件比较]
    E --> F[按写掩码更新深度与颜色]
```

OpenGL 规范定义比较与写入规则；驱动负责执行及可能的提前测试优化；GLFW 请求默认帧缓冲的附件；应用选择投影矩阵、清除时机和绘制顺序。没有深度附件时，即使启用深度测试也无法获得遮挡效果。

## 比较、清除与写入

`GL_LESS` 是默认比较函数：新深度必须严格小于已有深度。`GL_LEQUAL` 允许相等，后面的天空盒会用到它；`GL_ALWAYS` 始终通过，但在允许写入时仍会更新深度，不能把它与关闭测试完全等同。

下面摘录本例初始化中的比较函数与显示模式位置缓存：

```c++
    glDepthFunc(GL_LESS);
    const GLint depth_mode_uniform{uniform_location(program, "depth_mode")};
    if (depth_mode_uniform < 0) {
        return EXIT_FAILURE;
    }
    int depth_mode{options.initial_mode};
    bool previous_space{false};
```

每帧使用 `GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT` 清除旧记录。默认清除深度为 1；若关闭了深度写入，清除深度也会受到影响，必须先恢复写掩码。关闭深度写入的透明物体仍然可以与不透明物体的深度比较，详见后面的混合一节。

> **常见误解：** 所有深度附件都是“24 位 float”。
> **纠正：** 位宽与编码是两件事。`GL_DEPTH_COMPONENT24` 是归一化定点格式，`GL_DEPTH_COMPONENT32F` 才明确表示浮点格式。本章离屏示例使用 `GL_DEPTH24_STENCIL8`。默认窗口附件由窗口系统选定，不能仅凭请求的位数推断浮点编码。

## 透视深度与可视化

对本例的标准透视矩阵、默认深度范围和正的观察空间前向距离 $z$，窗口深度 $d$ 满足：

$$d=\frac{f}{f-n}-\frac{fn}{(f-n)z}$$

因此 $d=0.5$ 并不对应近平面和远平面的中点。把 $d$ 还原为 NDC 的 $2d-1$，再反解得到 $z$，才能直观观察线性距离。完整片段着色器逐字摘自本例：

```c++
constexpr const char* fragment_shader_source{R"glsl(
#version 330 core
in vec3 local_pos;
out vec4 frag_color;
uniform vec4 object_color;
uniform int depth_mode;
void main()
{
    float pattern = mod(floor((local_pos.x + 0.5) * 8.0) +
                        floor((local_pos.z + 0.5) * 8.0), 2.0);
    if (depth_mode == 0) {
        frag_color = vec4(object_color.rgb * mix(0.72, 1.0, pattern), object_color.a);
    } else {
        float depth = gl_FragCoord.z;
        if (depth_mode == 2) {
            float z_ndc = depth * 2.0 - 1.0;
            depth = (2.0 * 0.1 * 100.0) / (100.0 + 0.1 - z_ndc * (100.0 - 0.1));
            depth /= 100.0;
        }
        frag_color = vec4(vec3(depth), 1.0);
    }
}
)glsl"};
```

空格依次切换表面颜色、原始深度和线性深度除以远平面后的灰度；背景仍使用清除颜色。线性模式在当前场景较暗，是因为物体距离远小于远平面的 100 个单位。

![](../img/04/01/depth_non_linear_graph.png)

上图为参考教程的透视深度示意，不是本例运行截图。

## 深度冲突

共面或极近的表面可能落入相同的离散深度区间，出现 Z-fighting。优先避免重复共面表面，并在允许的视野范围内增大近平面距离、缩小远近平面比值；贴花等场景可以研究多边形偏移。提高格式精度也有帮助，但不能保证解决所有重叠表面问题。

> **进阶（提前深度测试）：** **逻辑上的片段操作顺序不等于驱动必须采用的执行时序。** 对最终可见结果没有影响时，驱动可以提前剔除被遮挡片段，节省片段着色工作；写 `gl_FragDepth`、`discard` 等行为会影响可进行的优化。GLSL 3.30 没有通用的 `early_fragment_tests` 布局限定符，本节既不依赖提前测试，也不以固定的硬件调度顺序解释正确性。

> **进阶（精度与投影）：** **本节公式只适用于这里的投影与默认深度范围。** 正交投影的深度关系不同；修改深度范围、投影矩阵或采用反向深度后，也应重新推导线性化公式。不要把深度值直接解释成欧氏距离，更不要把本例中写死的 0.1 和 100 套用到另一套相机裁剪面。

## 本仓库示例

示例源码：[`apps/04_advanced_opengl/01_depth_testing/main.cpp`](../../apps/04_advanced_opengl/01_depth_testing/main.cpp)。代码片段均原样取自此文件；原站概念图片与本例实测截图分别标明。

首次安装依赖与工具链要求见[构建指南](../build.md)。已安装默认 MinGW GCC Debug 依赖时，在仓库根目录执行：

```powershell
cmake --preset mingw-gcc-debug
cmake --build --preset mingw-gcc-debug --target 04_advanced_opengl__01_depth_testing
.\build\mingw-gcc-debug\apps\04_advanced_opengl\01_depth_testing\04_advanced_opengl__01_depth_testing.exe
```

Esc 退出；W/A/S/D 移动，鼠标转向，滚轮修改 FOV。空格切换表面颜色、原始深度和线性深度。

本仓库实测截图（默认模式）：

![](../img/04/01/01_depth_testing.png)

本章示例还支持 `--frames 3` 隐藏窗口运行指定帧数、`--mode M` 选择初始模式；配合 `--capture output.ppm` 可保存最后一帧。参数与检查方式详见[构建指南的高级 OpenGL 示例说明](../build.md#高级-opengl-示例)。

## 本章整体回顾

- **局部**：将投影得到的窗口深度与附件比较，由清除和写掩码维护后续比较的基础。
- **整体**：深度比较为后续模板、透明和离屏场景提供遮挡基础。

## 核校参考

- [OpenGL 3.3 Core 规范](https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf)：深度测试、写掩码及深度附件格式。
- [GLSL 3.30 规范](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.3.30.pdf)：阶段输入输出、内建变量及本例使用的语言语义。

下一节：[模板测试](02_stencil_testing.md)
