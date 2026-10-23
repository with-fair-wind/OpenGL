# 第 06 章：PBR

本章接续高级光照，按理论 → 直接光照 → 漫反射 IBL → 镜面 IBL 学习。教程参考 [LearnOpenGL-CN](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN) 与英文原文修订。本地沿用英文主线顺序编号为 06，对应中文底本的第 07 章；内容对应与修订记录见 [覆盖清单](coverage.md)。

| 篇目 | 核心内容 | 实际目标/模式 |
| --- | --- | --- |
| [01 理论](01_theory.md) | 微表面、能量、辐射度量、BRDF 与材质 | 概念篇；摘录标注后续真实源码，数值求积另标伪代码 |
| [02 光照](02_lighting.md) | 四点光源、GGX/Smith/Schlick、五贴图 | `06_pbr__02_lighting`，5 模式 |
| [03 漫反射辐照度](03_diffuse_irradiance.md) | HDR、六面转换、E/π 卷积与间接光 | `06_pbr__03_diffuse_irradiance`，5 模式 |
| [04 镜面反射 IBL](04_specular_ibl.md) | GGX 预过滤、LUT、五材质、模型和双环境 | `06_pbr__04_specular_ibl`，13 模式 |

三个独立 C++23/OpenGL 3.3 示例沿用匿名命名空间 helper，不新增公共库或 Conan 依赖。图片、材质与 HDR 都在仓库内，参见[图片记录](../img/06/README.md)与[运行资源来源](../../assets/chapter06_resources.md)。实际示例中的摘录逐字对应相应 main.cpp，作者、翻译和校对信息保留在各篇开头。

```mermaid
flowchart LR
    A[理论与单位] --> B[直接光源有限和]
    B --> C[环境的漫反射积分]
    C --> D[镜面分割求和]
    D --> E[多材质与模型]
```

构建与运行命令见[构建指南](../build.md#pbr-示例)。Esc 退出，WASD/鼠标移动视角，滚轮修改 FOV，空格切换模式。`--frames N` 隐藏窗口运行指定帧数，`--mode M` 选择模式，`--capture path.ppm` 保存最后一帧；IBL 预计算发生在进入渲染循环之前。

代码已在 MinGW GCC Debug、GTX 1660 运行 23/23 模式；内容、数值核验及未验证范围分开记录于[覆盖清单](coverage.md)。没有把局部探针、离线缓存、其他显卡/工具链、颜色管理或性能基准冒称为已验证。

下一节：[理论](01_theory.md)。本章之后接续实践专题，尚未创建相应章节。
