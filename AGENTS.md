# Repository Guidelines

## 项目概述

OpenGL Lab 是一个基于 C++23、CMake ≥ 3.28 和 Conan 2 的 LearnOpenGL（http://learnopengl.com）学习仓库，目标平台为 Windows。每个示例是独立可执行程序，按教程章节分组。当前代码进度：`01_getting_started`（7 例 + 03_shaders 3 个练习目标）、`02_lighting`（6 例）、`03_model_loading`（3 例）、`04_advanced_opengl`（11 例、48 个模式）与 `05_advanced_lighting`（10 例、63 个模式）；现有配套教程文件共 40 篇。第 04 章按官方中文上游固定提交 `331560886468e28f28326429334fc48470f9c27b` 重修，74 项标题及非标题教学内容对应见 `docs/04_advanced_opengl/coverage.md`；第 05 章使用同一上游提交，68 项标题及非标题内容对应见 `docs/05_advanced_lighting/coverage.md`；前 01–03 章尚需按加强后的逐项契约另行核验。无 CI、无测试框架（尚未配置）。

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
- `apps/04_advanced_opengl/` — 高级 OpenGL 11 例、48 模式：深度测试、双物体模板轮廓、RGBA 草/玻璃、面剔除、完整卷积后处理、照片天空盒/模型环境映射、高级缓冲数据、内建变量/四程序 UBO、几何房子/模型爆破/法线、矩阵实例化小行星、默认与离屏 MSAA/逐样本解析。helper 仍复制在匿名命名空间；复用已有 stb/Assimp 包，不新增 Conan 包。支持 `--frames N` 隐藏窗口冒烟、`--mode M` 选择初始模式、`--capture path.ppm` 保存末帧；来源见 `docs/img/04/README.md` 与 `assets/chapter04_resources.md`。
- `apps/05_advanced_lighting/` — 高级光照 10 例、63 模式：Blinn-Phong、Gamma/sRGB 路径、二维与点光源阴影、Assimp 切线材质、视差/POM、HDR、MRT 泛光、G 缓冲与球形光体积、SSAO。helper 仍逐例复制，无新 Conan 包；05/09/10 使用 Assimp。`--frames/--mode/--capture` 与第四章一致，06 的 Q/E 调深度尺度、07/08 调曝光。资源见 `assets/chapter05_resources.md` 与 `docs/img/05/README.md`。
- `cmake/` — `OpenGLLabOptions.cmake`：公共选项、C++23 标准、警告标志、clangd compile_commands 镜像目标。
- `conan/` — `conanfile.py` 在仓库根（不在 conan/ 下）；`conan/profiles/` 存 7 个 Conan profile（mingw-gcc、mingw-clang、msvc-ninja、msvc-ninja-multi、clang-cl、clang-cl-ninja-multi、msvc-vs2026）。
- `assets/textures/` — 3 个 PPM、草叶/玻璃 PNG 与 `skybox/` 六张 JPG；`assets/models/` — `crate/`（单 mesh 立方体 + 木箱贴图）、`stage/`（地面/箱子/八面体 3 mesh 3 材质，八面体无贴图演示回退）、`asteroid/`（自制行星/岩石 OBJ，供矩阵实例化）。第 04 章资源来源见 `assets/chapter04_resources.md`；`assets/textures/advanced_lighting/` 有第 05 章的 9 个材质图片，`assets/models/relief_box/` 复用手写箱子几何并关联法线材质，来源见 `assets/chapter05_resources.md`。
- `docs/<章节>/` — 分章中文教程（`01_getting_started`、`02_lighting`、`03_model_loading`、`04_advanced_opengl`、`05_advanced_lighting`），每章 README 索引 + 篇章 md；`docs/img/<NN>/<篇>/` 存章节图片（含本仓库示例截图）。
- `docs/build.md` — 权威构建参考：完整 preset 矩阵、逐工具链命令、多配置生成器说明、运行时行为（Esc/WASD/鼠标/滚轮 FOV）。
- `src/`、`tests/`、`include/` 当前不存在，勿假设存在。

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

以下契约适用于已有教程的修订和后续章节的新增。目录与篇数只说明文件已落地，不代表完整性验收；第 04 章本次重修的核验范围和边界见覆盖清单，后续继续以该清单维护，前 01–03 章需另行逐项核验，不能直接沿用“已完成”的结论。

### 上游对齐与内容完整性（必须）

- **权威底本**：以 [LearnOpenGL-CN/LearnOpenGL-CN](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN) 的对应章节为中文底本；`with-fair-wind/LearnOpenGL-CN` 等 fork 可作辅助对照，不能取代上游。开始新增或重修时记录所用上游分支、完整 commit SHA 和文件路径，按该明确版本验收；技术事实另以对应版本的英文原文和 GLFW/OpenGL/GLSL 规范核校。
- **逐篇、逐小节保留**：覆盖上游对应章节的全部教程篇目，以及每篇所有层级的正文小节、子小节、练习、译注和附加资源。保留其教学顺序与层级关系；上游一篇有多个 H1 时，可按本仓库单 H1 格式规范化标题级别，但必须保留这些标题对应的内容与相对层级。标题可纠正术语、错字和排版，不能借改名、合并或降级隐藏内容缺口。
- **内容同等覆盖**：每个对应小节必须保留其概念、解释、推导步骤、公式、API 参数与状态含义、案例过程、注意事项及练习要求。允许修正错误、改善中文表达和适配工程，不能将完整教程改写为知识概要、API 速查或简化示例说明。仅有同名标题或一句概述不算覆盖；字数、标题数量与文件数量也不能代替内容核验。
- **主案例不得降为练习**：上游正文展开的案例必须在本地同等展开，不能因现有程序尚未实现而删掉、缩成“延伸阅读”，或改称留给读者的练习。例如几何着色器的房子、爆破和法线可视化，实例化的小行星带与矩阵实例属性，帧缓冲的卷积、模糊和边缘检测，均属于应保留的教学内容。上游原有练习同样必须保留，不能拿它们替代被删掉的正文案例。
- **代码适配服从教学完整性**：以本仓库 C++23 实现替换代码表达时，保留原案例的教学目标、关键操作与可观察效果。缺少实现就补齐实现，可按现有独立示例模式增加目标或增加可切换模式；不能反过来只讲现有简化程序具备的功能。自制资源可以替代资源文件，但不能因此跳过原小节所教的加载、采样、变换或模型处理步骤。概念伪代码应标明用途与前置条件，不伪称源码摘录。
- **图片与公式完整**：复制并本地化上游各小节引用的教学图片，保留其说明作用和图文顺序，确保没有因漏图而丢失推导。允许按 GFM 重排图片链接、把公式图片转写为等价 LaTeX，并修正错误图注；不能仅选一张代表图代替一组逐步示意。本仓库实测截图另行标注，不能替代原教程用于解释原理的图片。保留作者、译者、校对者及来源信息。
- **纠错保留对应内容**：参考译文或英文原文有错误时，在原小节对应位置给出正确解释，必要时用简短修订说明与规范来源交代差异。过时 API 或工程组织需要现代化时，说明语义与实现对应关系；技术纠错不能成为删除相关小节或跳过教学过程的理由。

### 编写流程与覆盖清单

1. **先盘点再编写**：新增或重修章节前，读取上游全部对应文章，提取完整标题树，并盘点正文案例、公式、图片、提示块、练习与附加资源，不能只读目录或摘要。
2. **建立逐项映射**：在 `docs/<章节>/coverage.md` 记录上游版本、每篇文件路径，以及“上游小节 → 本地小节 → 案例/源码目标或模式 → 图片/公式 → 状态与修订说明”的覆盖清单。所有上游条目都必须有去向；本仓库新增的全景图、整体回顾等单独标明为补充，不能充当上游条目的替代项。
3. **按原顺序逐节落实**：先保证原教程内容和案例完整，再加入本仓库代码解释、职责边界、全景图与进阶补充。代码适配造成表达变化时，同步调整说明与截图，并在覆盖清单中记录对应关系。
4. **分别验收内容与实现**：内容验收检查是否遗漏、缩写或改变原小节教学目标；实现验收检查代码引用、构建、运行和视觉效果。任一项未通过，均明确标记为待补齐，不能用构建成功、截图存在或链接可解析宣称整章完成。

### 格式、代码引用与讲解规范

- **命名**：`docs/<章节>/<NN>_<slug>.md`（snake_case，与 `apps/` 目录一致）；每章一个索引 `README.md`；图片放 `docs/img/<章节>/<NN>/`，文中用相对路径 `![](../img/<章节>/<NN>/xxx.png)`。
- **结构**：H1 中文标题 → 元数据表（原文/作者/来源，示例篇加"本仓库示例"行）→ 正文 → `## 本仓库示例`（示例路径 + 构建/运行命令，仅示例篇）→ `## 本章整体回顾` → 底部"下一节"链接（链式串联；末篇指向下一章节，章节未写时用纯文本指引而非断链）。
- **正文结构**：上述外层结构中的“正文”必须容纳上游的完整小节树；本仓库的全景说明、常见误解和进阶块就近补入，不得用统一模板覆盖或取代上游原有讲解。
- **格式**：纯 GitHub Markdown，禁止 mkdocs 语法。转换规则：`!!! X` → `> **X：**`；`<fun>x</fun>`/`<var>x</var>` → `` `x` ``；`<def>术语</def>` → `**术语**`；LaTeX `$...$` 保留（GitHub 原生渲染）。
- **代码**：实际示例的代码片段必须**逐字取自** `apps/` 下对应 `main.cpp`（C++23 风格：snake_case、nullptr、花括号初始化、`F`/`U` 后缀）；同一篇涉及多个案例时，逐段标明各自源码和目标/模式——第二章起与第一章 `06_textures` 按"原样逐字"（含缩进与注释）执行；第一章其余示例篇允许"内容一致（含注释）、展示层缩进/换行可重排"的引用（历史排版约定）。独立概念片段或伪代码须明确标注，不受“源码摘录”冒称；其中可执行的通用代码需现代化（nullptr、返回值检查等）。源码摘录约束不允许通过删掉尚无对应实现的小节来满足。
- **修正与完善**：以 LearnOpenGL-CN 为底本时修正术语与病句；每篇融入"局部→整体"讲解——一句话核心（关键概念先给加粗核心句）、职责边界（OpenGL 规范 / GLFW / 显卡驱动 / 应用各自负责什么）、分层解释（规范→驱动→运行时加载；状态设置 vs 状态使用）、mermaid/ASCII 全景链路图、`> **常见误解：**` 块。
- **准确性核校**：技术描述以 [learnopengl.com](https://learnopengl.com) 英文原文为基准逐篇核校，修正译文中的术语错误、病句与松散表述；英文原文自身不精确处（如 "main context"）按 GLFW/OpenGL 规范术语纠正（→"当前上下文"）并保留简短说明。
- **进阶扩展**：对可自然延伸的概念补充 `> **进阶（主题）：**` 块（如多线程渲染、纹理单元、深度精度、透视除法、uniform 位置缓存），每篇 1–3 处、每处 100–200 字，先给加粗结论再展开；事实须经 GLFW/OpenGL 规范核对，不虚构 API；不喧宾夺主、不改变既有结构与链接。
- **术语统一**：Esc = 退出键（非返回键）；"链接器"（非连接器）；渲染/呈现统一用"渲染"；OpenGL 上下文（非"环境"）。

### 交付验收（全部满足才能声明完成）

- **上游覆盖**：覆盖清单包含指定版本的全部篇目及各级小节，映射完整且无待补齐项；练习、译注与附加资源均有对应位置。逐项阅读核实教学内容，不只比较标题数量。
- **教学深度**：概念、公式推导、参数说明、分步案例和必要图示没有被概要化；上游正文案例没有被移作练习或仅保留简介。新增补充不抵消遗漏。
- **代码与效果**：实际示例片段与所标注源码逐字一致，引用完整、无截断调用；覆盖所有正文案例所需的程序目标或模式，并记录构建、运行与必要的视觉检查结果及未验证范围。
- **格式与资源**：无 mkdocs 残留语法，全部本地链接、锚点和图片可解析；上游教学图片与本仓库截图来源明确，作者/翻译/校对及修订依据可追溯；每篇有 ≥1 全景图、`## 本章整体回顾`，"下一节"链完整。
- **如实报告进度**：分开报告代码可运行状态与教程对齐状态；任一内容或实现条目缺失，就保留“待补齐”状态并列明缺口，不将现有的简化稿、篇数或构建结果描述为原教程已完整落地。
