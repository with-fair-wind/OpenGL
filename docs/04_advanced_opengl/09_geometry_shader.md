# 几何着色器

| 项目 | 内容 |
| --- | --- |
| 原文 | [Geometry Shader](https://learnopengl.com/Advanced-OpenGL/Geometry-Shader) |
| 作者 | JoeyDeVries |
| 来源 | [LearnOpenGL-CN](https://github.com/with-fair-wind/LearnOpenGL-CN/blob/new-theme/docs/04%20Advanced%20OpenGL/09%20Geometry%20Shader.md)，按本仓库代码整理修订 |
| 本仓库示例 | [`apps/04_advanced_opengl/09_geometry_shader/`](../../apps/04_advanced_opengl/09_geometry_shader/) |

**几何着色器的一次调用处理一个输入图元，并向后续光栅化阶段发射新的点、线带或三角形带。** 本例把四个点各扩展成五顶点三角形带，绘制四座带白色屋顶的小房子。

## 新阶段的位置

```mermaid
flowchart LR
    A[四个 CPU 点顶点] --> B[顶点着色器]
    B --> C[每点一次几何着色器调用]
    C --> D[每次发射五个顶点]
    D --> E[三角形带 光栅化]
    E --> F[片段颜色插值]
```

应用提交 `GL_POINTS` 并编译/链接几何阶段；GLSL 规定输入图元、输出类型及最大顶点数；驱动执行阶段并检查接口。没有几何着色器时，顶点处理直接进入后续固定功能管线，几何阶段并非绘图必需。

## 输入数组与输出发射

输入 `gl_in` 是一个图元的顶点数组。points 有一个输入顶点；lines 有两个；triangles 有三个。邻接图元还有额外邻接顶点，不应把这些数量理解为整个 draw call 的顶点数。

```c++
constexpr const char* geometry_shader_source{R"glsl(
#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 5) out;
in vec3 point_color[];
out vec3 vertex_color;
void emit_corner(vec2 offset, vec3 color)
{
    gl_Position = gl_in[0].gl_Position + vec4(offset, 0.0, 0.0);
    vertex_color = color;
    EmitVertex();
}
void main()
{
    emit_corner(vec2(-0.16, -0.16), point_color[0]);
    emit_corner(vec2( 0.16, -0.16), point_color[0]);
    emit_corner(vec2(-0.16,  0.12), point_color[0]);
    emit_corner(vec2( 0.16,  0.12), point_color[0]);
    emit_corner(vec2( 0.00,  0.30), vec3(1.0));
    EndPrimitive();
}
)glsl"};
```

输出的 `triangle_strip` 每加入一个顶点都可能形成后续三角形。`max_vertices = 5` 是本次调用允许的输出上限；超过上限的行为未定义，不能假定驱动会安全截去多余顶点。

![](../img/04/09/geometry_shader_triangle_strip.png)

## 每次 EmitVertex 都要写全输出

本例把位置和颜色赋值放在 `emit_corner` 内，每次 `EmitVertex` 前都重新设置。发射后输出变量值未定义，不应沿用参考译文中“颜色只写一次，后面的顶点自动保留”的说法。

```c++
    const GLuint program{create_program(resources, vertex_shader_source, fragment_shader_source,
                                        geometry_shader_source)};
```

此例直接在裁剪位置上添加固定偏移，且所有输入点的 w=1，因此是屏幕式二维演示。要让房子在透视场景中保持世界尺寸，应在世界/观察空间生成顶点，最后再投影；不能把固定裁剪偏移直接当作固定世界长度。

> **常见误解：** GPU 生成顶点一定比 CPU 创建顶点并实例化更快。
> **纠正：** 几何阶段有输出上限和硬件吞吐约束，扩增工作也有成本。本例用于理解图元级处理，不作为草地或体素场景的通用性能结论。

## 爆破与法线可视化的延伸

原教程的爆破先为每个三角形求面法线，再沿法线平移其顶点；法线可视化则输出短线段。两者都要求位置与法线处于同一坐标空间，并在变换非均匀缩放时正确使用法线矩阵。本仓库当前目标只实现房子扩增，这两个效果留作本节扩展练习，不将原站背包效果图当作本例输出。

> **进阶（输出的生命周期）：** **几何阶段输出是一次发射的值快照，不是持续共享的可变对象。** 每个新顶点都要重新赋值其需要的输出成员；`EndPrimitive` 结束当前带并为下一段划定边界。阶段输出上限与总输出分量上限同时约束可用规模，应在设计扩增算法前查询能力，而不是仅把 max_vertices 调大来处理任意网格。

## 本仓库示例

示例源码：[`apps/04_advanced_opengl/09_geometry_shader/main.cpp`](../../apps/04_advanced_opengl/09_geometry_shader/main.cpp)。代码片段均原样取自此文件；原站概念图片与本例实测截图分别标明。

首次安装依赖与工具链要求见[构建指南](../build.md)。已安装默认 MinGW GCC Debug 依赖时，在仓库根目录执行：

```powershell
cmake --preset mingw-gcc-debug
cmake --build --preset mingw-gcc-debug --target 04_advanced_opengl__09_geometry_shader
.\build\mingw-gcc-debug\apps\04_advanced_opengl\09_geometry_shader\04_advanced_opengl__09_geometry_shader.exe
```

Esc 退出；本例使用固定的二维视图，不捕获鼠标。

本仓库实测截图（默认模式）：

![](../img/04/09/09_geometry_shader.png)

本章示例还支持 `--frames 3` 隐藏窗口运行指定帧数、`--mode M` 选择初始模式；配合 `--capture output.ppm` 可保存最后一帧。参数与检查方式详见[构建指南的高级 OpenGL 示例说明](../build.md#高级-opengl-示例)。

## 本章整体回顾

- **局部**：以输入图元为单位生成三角形带，每次发射前完整赋值输出。
- **整体**：图元扩增与重复实例是不同机制，下一节用实例属性降低重复提交。

## 核校参考

- [OpenGL 3.3 Core 规范](https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf)：几何阶段的图元接口与实现限制。
- [GLSL 3.30 规范](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.3.30.pdf)：几何着色器输出在 EmitVertex 后未定义，超出 max_vertices 的发射结果也未定义。

下一节：[实例化](10_instancing.md)
