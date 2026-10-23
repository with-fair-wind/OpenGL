# OpenGL Lab

这是一个面向 OpenGL 新手的现代 C++ 学习仓库，目标平台为 Windows。代码路线主要参考 [LearnOpenGL](https://learnopengl.com/)，中文教程基于 [LearnOpenGL-CN 官方上游](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN) 整理修订，并结合规范修正技术表述。当前已有入门、光照、模型加载、高级 OpenGL、高级光照和 PBR 六个章节的代码与教程；第 04、05、06 章的逐节对应范围和验证结果分别见[高级 OpenGL](docs/04_advanced_opengl/coverage.md)、[高级光照](docs/05_advanced_lighting/coverage.md)与 [PBR 覆盖清单](docs/06_pbr/coverage.md)，前 01–03 章需另按加强后的完整性规范核验。

## 当前目标

- 使用 CMake 管理构建。
- 使用 `CMakePresets.json` 固定常用构建入口。
- 使用 Conan 2 获取第三方依赖。
- 使用 C++23 编写示例和后续封装代码。
- OpenGL 学习路径先从 OpenGL 3.3 Core Profile 开始。

## 当前进度与教程入口

| 章节 | 中文教程 | 可执行示例 |
| --- | --- | --- |
| 01 入门 | [10 篇教程](docs/01_getting_started/README.md) | 7 个主示例 + 3 个着色器练习 |
| 02 光照 | [6 篇教程](docs/02_lighting/README.md) | 6 个主示例 |
| 03 模型加载 | [3 篇教程](docs/03_model_loading/README.md) | 3 个主示例 |
| 04 高级 OpenGL | [11 篇教程与覆盖清单](docs/04_advanced_opengl/README.md) | 11 个主示例，48 个模式 |
| 05 高级光照 | [10 篇教程与覆盖清单](docs/05_advanced_lighting/README.md) | 10 个主示例，63 个模式 |
| 06 PBR | [4 篇教程与覆盖清单](docs/06_pbr/README.md) | 3 个主示例，23 个模式；理论篇无独立目标 |

合计 44 篇教程、40 个主示例和 3 个练习目标。教程使用 GitHub Markdown，图片存放在 `docs/img/`；实际示例代码片段对应 `apps/` 源码，概念示例另行标明。PBR 之后接续实践专题，尚未创建对应教程和示例。

## 依赖

Conan 当前管理这些库：

- `glfw`: 创建窗口、OpenGL 上下文、处理输入。
- `glad`: 加载 OpenGL 函数。
- `glm`: 图形数学库。
- `stb`: 加载图片纹理。
- `assimp`: 导入模型、节点、网格和材质数据。

## 项目结构

```text
.
|-- apps/                    # 按 LearnOpenGL 章节组织的可执行示例
|   |-- 01_getting_started/
|   |   |-- 01_hello_window/
|   |   |-- 02_hello_triangle/
|   |   |-- 03_shaders/
|   |   |-- 04_textures/
|   |   |-- 05_transformations/
|   |   |-- 06_coordinate_systems/
|   |   `-- 07_camera/
|   |-- 02_lighting/
|   |   |-- 01_colors/
|   |   |-- 02_basic_lighting/
|   |   |-- 03_materials/
|   |   |-- 04_lighting_maps/
|   |   |-- 05_light_casters/
|   |   `-- 06_multiple_lights/
|   |-- 03_model_loading/
|   |   |-- 01_assimp/
|   |   |-- 02_mesh/
|   |   `-- 03_model/
|   |-- 04_advanced_opengl/   # 11 例：深度测试至离屏 MSAA
|   |-- 05_advanced_lighting/ # 10 例：Blinn-Phong 至 SSAO
|   `-- 06_pbr/               # 3 例：直接光照、漫反射与镜面 IBL
|-- assets/                  # 纹理与模型；GLSL 内嵌在示例源码中
|   |-- textures/
|   `-- models/
|-- cmake/                   # 项目级 CMake helper
|-- conan/                   # 本仓库 Conan profile
|-- docs/                    # 分章节中文教程与构建指南
|   |-- 01_getting_started/
|   |-- 02_lighting/
|   |-- 03_model_loading/
|   |-- 04_advanced_opengl/
|   |-- 05_advanced_lighting/
|   |-- 06_pbr/
|   |-- img/
|   `-- build.md
|-- CMakeLists.txt           # 根构建入口
|-- CMakePresets.json        # 常用 configure/build preset
`-- conanfile.py             # Conan 2 依赖声明
```

每个示例独立维护一个 `main.cpp` 和一个 `CMakeLists.txt`。窗口、Shader、纹理等辅助代码保留在各文件的匿名命名空间中，便于逐篇学习和对照；当前没有共享库、`src/`、`include/` 或 `tests/` 目录。

## 构建

完整构建矩阵、Conan 安装方式、单配置/多配置生成器说明，以及运行示例的命令见 [构建指南](docs/build.md)。

当前以 configure + build 作为冒烟检查，尚未接入 CTest 测试用例、测试框架或 CI。

## 学习路线

第一阶段按以下顺序学习已实现的入门示例：

1. 创建窗口 / 你好，窗口：`apps/01_getting_started/01_hello_window/`
2. 你好，三角形：`apps/01_getting_started/02_hello_triangle/`
3. 着色器：`apps/01_getting_started/03_shaders/`
4. 纹理：`apps/01_getting_started/04_textures/`
5. 变换：`apps/01_getting_started/05_transformations/`
6. 坐标系统：`apps/01_getting_started/06_coordinate_systems/`
7. 摄像机：`apps/01_getting_started/07_camera/`

第二阶段进入光照：

1. 颜色：`apps/02_lighting/01_colors/`
2. 基础光照：`apps/02_lighting/02_basic_lighting/`
3. 材质：`apps/02_lighting/03_materials/`
4. 光照贴图：`apps/02_lighting/04_lighting_maps/`
5. 投光物：`apps/02_lighting/05_light_casters/`
6. 多光源：`apps/02_lighting/06_multiple_lights/`

第三阶段进入模型加载：

1. Assimp 导入与节点树遍历：`apps/03_model_loading/01_assimp/`
2. Mesh 数据与 GPU 资源生命周期：`apps/03_model_loading/02_mesh/`
3. Model 多网格、多材质与贴图回退：`apps/03_model_loading/03_model/`

第四阶段进入[高级 OpenGL](docs/04_advanced_opengl/README.md)：深度测试 → 模板测试 → 混合 → 面剔除 → 帧缓冲 → 立方体贴图 → 高级数据 → 高级 GLSL → 几何着色器 → 实例化 → 抗锯齿。每节均有独立示例；键鼠、空格切换与有限帧运行方式见[构建指南](docs/build.md#高级-opengl-示例)。

继续扩展时沿用独立示例结构；需要引入可复用封装时，再单独设计公共模块及其测试。

第五阶段进入[高级光照](docs/05_advanced_lighting/README.md)：Blinn-Phong → Gamma → 二维/点光源阴影 → 法线/视差贴图 → HDR/泛光 → 延迟着色与 SSAO。10 个独立目标支持有限帧运行和按模式截图，教学图片、材质和验证边界见该章覆盖清单。

第六阶段进入 [PBR](docs/06_pbr/README.md)：微表面/辐射度量理论 → 直接光 BRDF → 漫反射环境卷积 → 镜面分割求和。五套真实材质、HDR 六面捕获、GGX 预过滤、BRDF LUT、模型导入与双环境均有对应代码或模式；上游编号差异和技术纠错见覆盖清单。
