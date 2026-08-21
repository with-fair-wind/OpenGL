# Getting Started 教程（01 章）

本目录是 OpenGL Lab 仓库的 Getting Started 章节中文教程，共 10 篇，对应 [LearnOpenGL](http://learnopengl.com) 的 Getting Started 部分。

内容基于 [LearnOpenGL-CN](https://github.com/with-fair-wind/LearnOpenGL-CN) 翻译整理，并做了两方面工作：

- **修正**：修正译文中术语不准确（如"连接器"→"链接器"、"返回键"→"退出键"）、病句、笔误等表述问题，统一专业术语。
- **完善**：从局部到整体的视角补充讲解——每个关键概念先给"一句话核心"，再明确职责边界（OpenGL 规范 / GLFW / 显卡驱动 / 应用程序各自负责什么），配全景链路图说明概念在整个渲染管线中的位置，并用本仓库实际代码逐段讲解。

## 篇章列表

| 篇 | 教程 | 本仓库示例 | 核心内容 |
|---|---|---|---|
| 01 | [OpenGL](01_opengl.md) | — | OpenGL 是什么、规范/实现/驱动分层、状态机、对象模型 |
| 02 | [创建窗口](02_creating_a_window.md) | — | GLFW/GLAD 的作用与职责边界；本仓库用 Conan 2 管理依赖 |
| 03 | [你好，窗口](03_hello_window.md) | `apps/01_getting_started/01_hello_window/` | 窗口创建、GLAD 初始化、视口、渲染循环、双缓冲 |
| 04 | [你好，三角形](04_hello_triangle.md) | `apps/01_getting_started/02_hello_triangle/` | 顶点输入、VBO/VAO、着色器编译、链接、绘制 |
| 05 | [着色器](05_shaders.md) | `apps/01_getting_started/03_shaders/` | GLSL 语言、顶点/片元着色器、uniform、顶点属性插值 |
| 06 | [纹理](06_textures.md) | `apps/01_getting_started/04_textures/` | 纹理坐标/采样/环绕/过滤、stb_image 加载、纹理单元 |
| 07 | [变换](07_transformations.md) | `apps/01_getting_started/05_transformations/` | GLM 数学库、平移/旋转/缩放、模型矩阵、时间驱动动画 |
| 08 | [坐标系](08_coordinate_systems.md) | `apps/01_getting_started/06_coordinate_systems/` | 局部→世界→观察→裁剪→屏幕坐标管线、MVP 矩阵、深度测试 |
| 09 | [摄像机](09_camera.md) | `apps/01_getting_started/07_camera/` | 观察矩阵、欧拉角、第一人称相机、delta_time、鼠标/滚轮回调 |
| 10 | [复习](10_review.md) | — | 全章知识回顾与学习路线串联 |

## 如何跟随学习

1. 按篇章顺序阅读；每篇末尾的"本章整体回顾"会说明该章在整个管线中的位置。
2. 有对应示例的篇章（03–09）在"本仓库示例"小节给出示例路径、构建与运行命令。
3. 构建与运行方式总览见 [`docs/build.md`](../build.md)（Conan 2 + CMake presets，默认 `mingw-gcc-debug`）。

运行示例（默认 MinGW GCC Debug）：

```powershell
conan install . -of build/mingw-gcc-debug -pr:h conan/profiles/mingw-gcc -pr:b conan/profiles/mingw-gcc -s build_type=Debug --build=missing
cmake --preset mingw-gcc-debug
cmake --build --preset mingw-gcc-debug
.\build\mingw-gcc-debug\apps\01_getting_started\03_shaders\01_getting_started__03_shaders.exe
```

示例通用操作：`Esc` 退出；相机示例（07 起）用 `W/A/S/D` 移动、鼠标调整视角、滚轮缩放 FOV。
