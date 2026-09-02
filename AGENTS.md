# Repository Guidelines

## 项目概述

OpenGL Lab 是一个基于 C++23、CMake ≥ 3.28 和 Conan 2 的 LearnOpenGL（http://learnopengl.com）学习仓库，目标平台为 Windows。每个示例是独立可执行程序，按教程章节分组。当前进度：`01_getting_started`（7 例 + 03_shaders 3 个练习目标）、`02_lighting`（6 例）与 `03_model_loading`（3 例）。无 CI、无测试框架（尚未配置）。

## 架构与数据流

- **无共享库/公共头文件**：每个示例 = 一个 `main.cpp` + 一个 `CMakeLists.txt`。GLFW/GLAD 初始化、`compile_shader`/`create_shader_program`、纹理 RAII 等 helper 全部复制粘贴在各文件匿名命名空间 `namespace { ... }` 内——这是刻意的教学方式。新增示例时沿用复制模式，不要擅自抽取共享代码到 `src/`（除非用户明确要求）。
- **渲染主循环**（所有示例一致）：`process_input` → `glClearColor`/`glClear` → `glUseProgram` → 设置 uniform → `glActiveTexture`/`glBindTexture`（纹理示例）→ `glBindVertexArray` → `glDrawArrays` → `glfwSwapBuffers` → `glfwPollEvents`。自 `06_coordinate_systems` 起启用 `GL_DEPTH_TEST`。
- **Shader 内嵌**：GLSL 以 `constexpr const char* ..._source{R"glsl(#version 330 core ...)glsl"}` 原始字符串嵌入，无 `.vert`/`.frag` 文件、无 ifstream。
- **纹理**：stb_image（`#define STB_IMAGE_IMPLEMENTATION` + `<stb_image.h>`），路径由编译期宏 `OPENGL_LAB_ASSET_ROOT`（指向 `assets/`，未定义时回退 `"."`）经 `std::filesystem` 拼接，`stbi_image_free` 由 `stbi_image_deleter` + `std::unique_ptr` RAII 管理。
- **矩阵**：GLM 列主序，`glm::value_ptr` + `glUniformMatrix4fv(..., GL_FALSE, ...)`；`glm::lookAt/perspective/rotate/scale/radians`。
- **数据流**：CPU 顶点数据 → VBO/VAO(EBO) → vertex shader → fragment shader → 帧缓冲 → 窗口。

## 关键目录

- `apps/01_getting_started/` — 入门章节 7 例：`01_hello_window`（无 OpenGL 对象）… `07_camera`（EBO/纹理/变换/坐标系/第一人称相机逐步引入）。
- `apps/02_lighting/` — 光照章节 6 例：`01_colors` … `06_multiple_lights`（Phong 光照、材质/光源 struct uniform、光照贴图、点光源衰减、多光源数组 uniform）。`03_shaders` 下另有 `exercise1-3.cpp` 练习目标。
- `apps/03_model_loading/` — 模型加载章节 3 例：`01_assimp`（导入 OBJ、打印 aiScene 统计、沿 aiNode 节点树递归渲染、法线可视化着色）、`02_mesh`（Vertex/Mesh 结构 + make/draw/destroy 函数封装）、`03_model`（Model 递归加载多 mesh/多材质，贴图相对模型目录解析，无贴图回退 1x1 纯色纹理）。
- `cmake/` — `OpenGLLabOptions.cmake`：公共选项、C++23 标准、警告标志、clangd compile_commands 镜像目标。
- `conan/` — `conanfile.py` 在仓库根（不在 conan/ 下）；`conan/profiles/` 存 7 个 Conan profile（mingw-gcc、mingw-clang、msvc-ninja、msvc-ninja-multi、clang-cl、clang-cl-ninja-multi、msvc-vs2026）。
- `assets/textures/` — 3 个 PPM 纹理：`checker.ppm`、`container_diffuse.ppm`、`container_specular.ppm`；`assets/models/` — 手写 OBJ/MTL 模型：`crate/`（单 mesh 立方体 + 木箱贴图，供 01_assimp）、`stage/`（地面/箱子/八面体 3 mesh 3 材质，供 03_model；八面体故意无贴图演示回退路径）。
- `docs/<章节>/` — 分章中文教程（`01_getting_started`、`02_lighting`、`03_model_loading`），每章 README 索引 + 篇章 md；`docs/img/<NN>/<篇>/` 存章节图片（含本仓库示例截图）。
- `docs/build.md` — 权威构建参考：完整 preset 矩阵、逐工具链命令、多配置生成器说明、运行时行为（Esc/WASD/鼠标/滚轮 FOV）。
- `src/`、`tests/`、`include/` 当前不存在（README 规划中，勿假设存在）。

## 开发命令

默认 MinGW GCC Debug（MSYS2 UCRT64，需 `ucrt64/bin` 在 PATH）：

```powershell
conan install . -of build/mingw-gcc-debug -pr:h conan/profiles/mingw-gcc -pr:b conan/profiles/mingw-gcc -s build_type=Debug --build=missing
cmake --preset mingw-gcc-debug
cmake --build --preset mingw-gcc-debug
```

运行示例（单配置布局；多配置生成器产物在 `build/<preset>/<config>/` 下）：

```powershell
.\build\mingw-gcc-debug\apps\01_getting_started\01_hello_window\01_getting_started__01_hello_window.exe
```

- 全部 preset：`cmake --list-presets=all` — 12 个 configure preset：Ninja 单配置 `mingw-gcc|mingw-clang|msvc-ninja|clang-cl` 各 debug/release；多配置 `msvc-ninja-multi`、`clang-cl-ninja-multi`、`msvc-vs2026`（build 时用 `--preset <name>-debug|-release`）。MSVC/clang-cl/VS2026 preset 须在 Visual Studio Developer PowerShell 中运行。
- 警告即错误（默认关闭）：`-DOPENGL_LAB_ENABLE_WARNINGS_AS_ERRORS=ON`（CMake cache 或 preset 覆写）。
- 依赖变更后重新执行 conan install；新增示例只需新目录 + `apps/<chapter>/CMakeLists.txt` 加一行 `add_subdirectory`。
- 修改 preset、profile 或构建步骤后必须同步更新 `docs/build.md`。

## 代码规范与常见模式

- **格式**：clang-format（Google 基础，4 空格缩进，100 列，指针/引用左对齐，include 按类分组排序）；clang-tidy 已启用 bugprone/cert/clang-analyzer/concurrency/cppcoreguidelines/modernize/performance/portability/readability 九族（18 项排除，命名约定、函数规模阈值）。改完代码用 `clang-format -i` 格式化。
- **命名**：函数/变量/局部 struct 一律 `snake_case`；`constexpr` 常量 `snake_case`（如 `window_width`、`mouse_sensitivity{0.10F}`）；宏 `SCREAMING_SNAKE`。float 字面量带 `F` 后缀、unsigned 带 `U` 后缀、初始化一律花括号 `{}`、局部常量用 `const`。
- **GLSL 命名**：attribute 前缀 `a_`（`a_pos`、`a_color`、`a_normal`、`a_tex_coord`）；struct PascalCase（`Material`、`PointLight`）、成员 `snake_case`。
- **include 顺序**（固定约定）：标准库（字母序）→ 空行 → `<glad/glad.h>`（必须先于 glfw3.h，GLAD 定义 GL 类型）→ 空行 → `<GLFW/glfw3.h>` → 空行 → glm 头（`glm.hpp`、`gtc/matrix_transform.hpp`、`gtc/type_ptr.hpp`）→ 空行 → `#define STB_IMAGE_IMPLEMENTATION` + `<stb_image.h>`（纹理示例）→ 空行 → assimp 头（`Importer.hpp`、`postprocess.h`、`scene.h`，模型加载示例）→ `namespace {`。
- **Doxygen**（中文）：文件头 `/** @file ... @brief ... @details ... */`；公开函数、GLFW 回调、重要常量用 `@brief`，必要时 `@details/@param/@return/@pre/@note/@see`。解释"为什么"和前置条件，避免复述代码。可 `@ref` 交叉引用同文件函数。
- **就近行内注释**：主题标签前缀 `// OpenGL:`、`// GLFW:`、`// GLM/OpenGL:`、`// Camera:`、`// stb_image:`，说明上下文、状态机、副作用、单位、资源生命周期或失败处理。**首次出现的 `gl*`/`glfw*`/`glad*` 调用必须注释**；自解释的普通 C++ 语句不加注释。
- **错误处理**：初始化失败 `std::cerr` + `return EXIT_FAILURE`；uniform 位置 -1 检查（getting_started 04-07 在 setup 缓存 `GLint`，lighting 章节在循环内联 `glGetUniformLocation`——沿用所在章节模式）；纹理加载失败打印路径。
- **CMake 叶子模式**：`add_executable(<target> main.cpp)` → `opengl_lab_apply_common_options(<target>)` → `target_link_libraries(<target> PRIVATE glad::glad glfw glm::glm-header-only)`；纹理示例追加 `stb::stb`，模型加载示例追加 `assimp::assimp`，两者均需 `target_compile_definitions(... OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets")`。注意 **`glfw` 是非命名空间目标**（不是 `glfw::glfw`）。
- **目标命名**：`<section>__<example>`（双下划线），如 `01_getting_started__04_textures`、`02_lighting__06_multiple_lights`。
- **提交**：简短祈使式摘要，可选 conventional 前缀（`chore:`/`feat:`/`fix:`）；PR 附摘要、影响的 preset/平台、已运行的验证命令；仅视觉输出变化才附截图。
- **Agent 专用**：不要提交 `build/`、`compile_commands.json`（镜像目标生成）、CMakeUserPresets.json 或 Conan 输出；工作区已有用户改动不要回滚，相关就先读懂再在其基础上继续。

## 重要文件

- `CMakeLists.txt`（根）— `include(OpenGLLabOptions)`、`opengl_lab_setup_options()`、`opengl_lab_setup_clangd_support()`、`find_package(glfw3|glad|glm|stb|assimp CONFIG REQUIRED)`（目标来自 Conan CMakeDeps）、`OPENGL_LAB_BUILD_EXAMPLES` 时 `add_subdirectory(apps)`。
- `cmake/OpenGLLabOptions.cmake` — 4 个选项：`OPENGL_LAB_BUILD_EXAMPLES`(ON)、`OPENGL_LAB_ENABLE_WARNINGS_AS_ERRORS`(OFF)、`OPENGL_LAB_WITH_QT`(OFF)、`OPENGL_LAB_WITH_OPENCV`(OFF)（后两者为 Qt/OpenCV 预留）；C++23（扩展关闭）；`-Wall -Wextra -Wpedantic`（MSVC：`/W4 /permissive-`）；`opengl_lab_mirror_compile_commands` 把 compile_commands.json 复制到源码根供 clangd。
- `conanfile.py`（**仓库根**）— `opengl_lab` v0.1.0，依赖 glfw/3.4、glad/0.1.36、glm/1.0.1、stb/cci.20240531（`force=True` 覆盖 assimp 传递的旧版 stb）、assimp/5.4.3；CMakeDeps + CMakeToolchain、`cmake_layout`。Conan 生成的 toolchain 在 `build/<preset>/build/<config>/generators/`。
- `CMakePresets.json` — 10 个 `_` 前缀隐藏基 preset + 12 个 configure preset + 14 个 build preset；`CMakeUserPresets.json`（gitignored）include Conan 生成的 toolchain preset。
- 示例参考：`apps/01_getting_started/07_camera/main.cpp`（相机状态 + 回调模式）、`apps/02_lighting/06_multiple_lights/main.cpp`（多光源数组 uniform）。

## 运行时/工具链偏好

- 目标平台 Windows；主开发工具链 MSYS2 MinGW GCC（UCRT64），其余 preset 需对应工具链就绪。C++23（GCC 扩展关闭），Conan profile 固定 `cppstd 23`。
- clangd：依赖源码根 `compile_commands.json`；`.clangd` 已配严格 include 诊断、clang-tidy 覆盖、inlay hints，并移除 MSVC 的 `/Fo*`/`/Fd*`/`/FS` 标志。
- 无 sanitizer 配置；无 CI；无脚本（除 conanfile.py）。
- 运行时交互约定（跨示例一致）：Esc 退出、WASD + 鼠标移动相机（07 起）、滚轮缩放 FOV、纹理加载自 `assets/textures/`、模型加载自 `assets/models/`（控制台打印场景统计）。Git Bash 下运行新构建 exe 若立即退出（退出码 127），是 Git 自带 `mingw64/bin` 抢占运行时 DLL 所致，需把 `ucrt64/bin` 提到 PATH 最前或改用 PowerShell。

## 测试与 QA

- **测试框架尚未配置**：无 `tests/` 目录、无 CTest 接入（无 `enable_testing`/`add_test`/`testPresets`）、无测试依赖。当前冒烟检查 = configure + build 成功。
- **约定**（新增可复用代码时启用）：在 `tests/` 下添加 `test_<feature>.cpp`，接入 CTest，用 `ctest --test-dir build/<preset>` 运行。
- `.gitignore` 已预留 `Testing/`、`.cache/` 忽略项。

## 教程文档规范（docs/ 分章教程）

`docs/01_getting_started/`、`docs/02_lighting/`、`docs/03_model_loading/` 是已完成的分章中文教程（基于 LearnOpenGL-CN 整理修订）；后续章节（04_advanced_opengl 等）沿用同一契约：

- **命名**：`docs/<章节>/<NN>_<slug>.md`（snake_case，与 `apps/` 目录一致）；每章一个索引 `README.md`；图片放 `docs/img/<章节>/<NN>/`，文中用相对路径 `![](../img/<章节>/<NN>/xxx.png)`。
- **结构**：H1 中文标题 → 元数据表（原文/作者/来源，示例篇加"本仓库示例"行）→ 正文 → `## 本仓库示例`（示例路径 + 构建/运行命令，仅示例篇）→ `## 本章整体回顾` → 底部"下一节"链接（链式串联；末篇指向下一章节，章节未写时用纯文本指引而非断链）。
- **格式**：纯 GitHub Markdown，禁止 mkdocs 语法。转换规则：`!!! X` → `> **X：**`；`<fun>x</fun>`/`<var>x</var>` → `` `x` ``；`<def>术语</def>` → `**术语**`；LaTeX `$...$` 保留（GitHub 原生渲染）。
- **代码**：有示例对应的篇目，代码片段必须**逐字取自** `apps/` 下对应 `main.cpp`（C++23 风格：snake_case、nullptr、花括号初始化、`F`/`U` 后缀）——第二章起与第一章 `06_textures` 按"原样逐字"（含缩进与注释）执行；第一章其余示例篇允许"内容一致（含注释）、展示层缩进/换行可重排"的引用（历史排版约定）。概念篇可保留通用教学代码但需现代化（nullptr、返回值检查等）。
- **修正与完善**：以 LearnOpenGL-CN 为底本时修正术语与病句；每篇融入"局部→整体"讲解——一句话核心（关键概念先给加粗核心句）、职责边界（OpenGL 规范 / GLFW / 显卡驱动 / 应用各自负责什么）、分层解释（规范→驱动→运行时加载；状态设置 vs 状态使用）、mermaid/ASCII 全景链路图、`> **常见误解：**` 块。
- **准确性核校**：技术描述以 [learnopengl.com](https://learnopengl.com) 英文原文为基准逐篇核校，修正译文中的术语错误、病句与松散表述；英文原文自身不精确处（如 "main context"）按 GLFW/OpenGL 规范术语纠正（→"当前上下文"）并保留简短说明。
- **进阶扩展**：对可自然延伸的概念补充 `> **进阶（主题）：**` 块（如多线程渲染、纹理单元、深度精度、透视除法、uniform 位置缓存），每篇 1–3 处、每处 100–200 字，先给加粗结论再展开；事实须经 GLFW/OpenGL 规范核对，不虚构 API；不喧宾夺主、不改变既有结构与链接。
- **术语统一**：Esc = 退出键（非返回键）；"链接器"（非连接器）；渲染/呈现统一用"渲染"；OpenGL 上下文（非"环境"）。
- **交付验证**：无 mkdocs 残留语法、全部图片引用可解析、代码块与 main.cpp 一致、每篇含 ≥1 全景图与 `## 本章整体回顾`、"下一节"链接链完整。
