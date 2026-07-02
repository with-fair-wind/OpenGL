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

`CMakePresets.json` 和 `conan/profiles/*` 不写本机绝对路径，要求对应 MSYS2 环境已经在 `PATH` 上：

- GCC preset: 需要 `ucrt64/bin`，并能找到 `x86_64-w64-mingw32-g++`。
- Clang preset: 需要 `clang64/bin`，并能找到 `x86_64-w64-mingw32-clang++`。

先安装依赖，再配置和编译。下面以 MinGW GCC Debug 为例：

```powershell
conan install . -of build/mingw-gcc-debug -pr:h conan/profiles/mingw-gcc -pr:b conan/profiles/mingw-gcc -s build_type=Debug --build=missing
cmake --preset mingw-gcc-debug
cmake --build --preset mingw-gcc-debug
```

Release 构建：

```powershell
conan install . -of build/mingw-gcc-release -pr:h conan/profiles/mingw-gcc -pr:b conan/profiles/mingw-gcc -s build_type=Release --build=missing
cmake --preset mingw-gcc-release
cmake --build --preset mingw-gcc-release
```

如果使用 Clang preset，把 `mingw-gcc-debug` 替换为 `mingw-clang-debug`，并把 Conan profile 替换为 `conan/profiles/mingw-clang`。

## 运行第一个示例

构建完成后运行：

```powershell
.\build\mingw-gcc-debug\apps\01_getting_started\01_hello_window\01_getting_started__01_hello_window.exe
```

窗口打开后，按 `Esc` 退出。

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
