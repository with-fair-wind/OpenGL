# OpenGL Lab

这是一个面向 OpenGL 新手的现代 C++ 学习仓库。代码路线主要参考 [LearnOpenGL CN](https://learnopengl-cn.github.io/)，工程组织会尽量保持清晰、可扩展，方便后续继续接入 Qt、OpenCV、模型加载、图像处理和小型渲染实验。

## 当前目标

- 使用 CMake 管理构建。
- 使用 `CMakePresets.json` 固定常用构建入口。
- 使用 Conan 2 获取第三方依赖。
- 使用 C++23 编写示例和后续封装代码。
- OpenGL 学习路径先从 OpenGL 3.3 Core Profile 开始。

## 依赖

Conan 当前管理这些库：

- `glfw`: 创建窗口、OpenGL 上下文、处理输入。
- `glad`: 加载 OpenGL 函数。
- `glm`: 图形数学库。
- `stb`: 后续加载图片纹理。

## 项目结构

```text
.
|-- apps/                    # 按 LearnOpenGL 章节组织的可执行示例
|   `-- 01_getting_started/
|       `-- 01_hello_window/
|-- cmake/                   # 项目级 CMake helper
|-- conan/                   # 本仓库 Conan profile
|-- docs/                    # 构建说明和项目文档
|-- CMakeLists.txt           # 根构建入口
|-- CMakePresets.json        # 常用 configure/build preset
`-- conanfile.py             # Conan 2 依赖声明
```

未来建议继续扩展：

```text
assets/                      # shader、纹理、模型等资源
src/opengl_lab/              # 可复用 C++ 封装
tests/                       # 单元测试或小型验证测试
third_party/                 # 仅放无法通过 Conan 管理的极少数依赖
```

## 构建

完整构建矩阵、Conan 安装方式、单配置/多配置生成器说明，以及运行第一个示例的命令见 [构建指南](docs/build.md)。

## 学习路线

第一阶段建议按这个顺序添加示例：

1. 创建窗口
2. 你好，窗口
3. 你好，三角形
4. 着色器
5. 纹理
6. 变换
7. 坐标系统
8. 摄像机

每个示例先保持独立可运行；当重复代码变多时，再把窗口、Shader、纹理、相机等内容提取到 `src/opengl_lab/` 中。
