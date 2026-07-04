# Repository Guidelines

## 项目结构与模块组织

OpenGL Lab 是一个基于 C++23、CMake Presets 和 Conan 2 的 OpenGL 学习仓库。可执行示例放在 `apps/` 下，并按 LearnOpenGL 章节分组，例如 `apps/01_getting_started/01_hello_window/`。项目级 CMake helper 放在 `cmake/`，Conan profile 放在 `conan/profiles/`，构建和协作文档放在 `docs/`。后续可复用渲染代码建议放入 `src/opengl_lab/`；shader、纹理、模型等资源放入 `assets/`；测试放入 `tests/`。

## 构建、测试与本地开发命令

配置 preset 前先安装依赖。默认 MinGW GCC Debug 路径示例：

```powershell
conan install . -of build/mingw-gcc-debug -pr:h conan/profiles/mingw-gcc -pr:b conan/profiles/mingw-gcc -s build_type=Debug --build=missing
cmake --preset mingw-gcc-debug
cmake --build --preset mingw-gcc-debug
```

运行第一个示例：

```powershell
.\build\mingw-gcc-debug\apps\01_getting_started\01_hello_window\01_getting_started__01_hello_window.exe
```

使用 `cmake --list-presets=all` 查看 MSYS2、MSVC、clang-cl、Ninja、Ninja Multi-Config 和 Visual Studio 相关 preset。完整构建矩阵见 `docs/build.md`。

## 代码风格与命名约定

使用 C++23，并按 `.clang-format` 格式化：Google 基础风格、4 空格缩进、不使用 tab、100 列限制、左大括号贴靠、include 区块按大小写敏感排序。局部 helper 优先使用 `snake_case`；CMake 目标名使用描述性命名，例如 `01_getting_started__01_hello_window`。

## Doxygen 与 OpenGL API 注释规范

新增或修改 C++ 示例时，生成中文 Doxygen 注释。文件、类/结构、公共函数、GLFW 回调、重要常量使用 `/** ... */` 文档块；优先包含 `@brief`，必要时补充 `@details`、`@param`、`@return`、`@pre`、`@note`、`@see`。注释应解释“为什么”和“前置条件”，避免只复述代码。

涉及 OpenGL、GLFW、GLAD 的 API 时，为初学者补充就近行内注释，说明上下文、状态机、副作用、单位、资源生命周期或失败处理。例如：

```cpp
// OpenGL: glClearColor 只设置清屏颜色状态，真正清空发生在 glClear。
glClearColor(0.10F, 0.14F, 0.18F, 1.0F);
```

首次引入 `gl*`、`glfw*`、`glad*` 调用时尤其要说明当前 OpenGL 上下文、缓冲区、对象绑定和双缓冲等概念。明显自解释的普通 C++ 语句不需要注释。

## 测试指南

当前尚未配置测试框架。现阶段将成功 configure 和 build 作为基础冒烟检查。新增可复用代码时，请在 `tests/` 下添加聚焦测试，文件命名为 `test_<feature>.cpp`，并接入 CTest，便于使用 `ctest --test-dir build/<preset>` 运行。

## Commit 与 Pull Request 规范

近期提交使用简短、祈使式摘要，部分使用 conventional prefix，例如 `chore: bootstrap opengl learning project` 或 `Add build presets and clangd support`。提交应保持范围清晰，并描述可观察的变更。PR 应包含简短摘要、影响的 preset 或平台、已运行的验证命令；只有视觉输出变化时才需要附截图。

## Agent 专用注意事项

不要提交 `build/`、`compile_commands.json` 或 Conan 输出等生成文件。修改 preset、profile 或构建步骤时，同步更新 `docs/build.md`。工作区已有用户改动时不要擅自回滚；如变更相关，先读懂并在其基础上继续修改。