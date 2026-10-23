# 第 02 章上游覆盖与核验清单

权威底本：`LearnOpenGL-CN/LearnOpenGL-CN`，分支 `new-theme`，固定提交 `331560886468e28f28326429334fc48470f9c27b`。本次盘点 7 篇、37 个各级标题，并记录非标题案例、公式、提示、练习、媒体与附加资源。以下清单集中记录来源版本和内容去向；教程正文只保留简洁署名。

**验收状态：未通过。** 本清单完成首轮逐项定位与缺口登记，不把“已定位”当成内容等价验收。标题数量、图片散列和源码匹配不能替代逐段教学深度核验；未完成的条目明确保留。

本轮实现验证：MinGW GCC Debug 全项目构建通过；01–03 章仍没有有限帧命令行入口，本轮未重新验证其完整交互与视觉。第 04–06 章已有的运行记录保留在各章覆盖清单，不新增仓库 QA 入口。

## 本次准确性修订

已补齐灯光着色器摘录中的空行，并为跨示例的镜面、定向光与聚光灯片段标注实际来源；将 shininess 解释为高光集中程度，不再等同于总反射强度。原有正文案例完整性缺口仍待补齐。

本次移除仓库 QA 入口和重复机器清单；来源版本只在此处集中维护。上述修正不改变其他“缺口 / 待复核”的状态。

## 状态约定

- **已核验**：明确核对过该项内容与对应技术事实；不外推到整章。
- **待复核**：找到对应位置，但尚不能证明概念、推导与案例同等完整。
- **缺口**：确认缺少小节、案例、实现或原教学层级，必须补齐后重新验收。

本仓库新增的全景图、职责说明、进阶块与整体回顾均为补充，不替代底本条目。

## 01 · 颜色

上游：[docs/02 Lighting/01 Colors.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/02%20Lighting/01%20Colors.md)；本地：[01_colors.md](01_colors.md)；源码目标：`02_lighting__01_colors`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 颜色 | 颜色 | 待复核 |
| H1 · 创建一个光照场景 | 创建一个光照场景 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 光色与反射率相乘、灯体与受光物体 | 创建一个光照场景；`02_lighting__01_colors`。01_colors；正文主线与图片定位，内容等价与视觉待复核。 | 待补齐或复核 |

媒体：2 次上游引用；公式：0 项；提示/译注：1 项；资源与练习链接：0 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 颜色 | [light_reflection.png](../img/02/01/light_reflection.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 创建一个光照场景 | [colors_scene.png](../img/02/01/colors_scene.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 02 · 基础光照

上游：[docs/02 Lighting/02 Basic Lighting.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/02%20Lighting/02%20Basic%20Lighting.md)；本地：[02_basic_lighting.md](02_basic_lighting.md)；源码目标：`02_lighting__02_basic_lighting`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 基础光照 | 基础光照 | 待复核 |
| H1 · 环境光照 | 环境光照 | 待复核 |
| H1 · 漫反射光照 | 漫反射光照 | 待复核 |
| H2 · 法向量 | 法向量 | 待复核 |
| H2 · 计算漫反射光照 | 计算漫反射光照 | 待复核 |
| H2 · 最后一件事 | 最后一件事：法线矩阵 | 待复核 |
| H1 · 镜面光照 | 镜面光照 | 待复核 |
| H2 · 练习 | 练习 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 环境、漫反射、法线矩阵、镜面反射及四项原练习 | 镜面光照；`02_lighting__02_basic_lighting`。02_basic_lighting；补回环境光单独效果图，全部中间案例待验收。 | 待补齐或复核 |

媒体：9 次上游引用；公式：2 项；提示/译注：6 项；资源与练习链接：4 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 基础光照 | [basic_lighting_phong.png](../img/02/02/basic_lighting_phong.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 环境光照 | [ambient_lighting.png](../img/02/02/ambient_lighting.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 漫反射光照 | [diffuse_light.png](../img/02/02/diffuse_light.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 计算漫反射光照 | [basic_lighting_diffuse.png](../img/02/02/basic_lighting_diffuse.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 最后一件事 | [basic_lighting_normal_transformation.png](../img/02/02/basic_lighting_normal_transformation.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 镜面光照 | [basic_lighting_specular_theory.png](../img/02/02/basic_lighting_specular_theory.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 镜面光照 | [basic_lighting_specular_shininess.png](../img/02/02/basic_lighting_specular_shininess.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 镜面光照 | [basic_lighting_specular.png](../img/02/02/basic_lighting_specular.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 镜面光照 | [basic_lighting_gouruad.png](../img/02/02/basic_lighting_gouruad.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 03 · 材质

上游：[docs/02 Lighting/03 Materials.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/02%20Lighting/03%20Materials.md)；本地：[03_materials.md](03_materials.md)；源码目标：`02_lighting__03_materials`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 材质 | 材质 | 待复核 |
| H1 · 设置材质 | 设置材质 | 待复核 |
| H2 · 光的属性 | 光的属性 | 待复核 |
| H2 · 不同的光源颜色 | 不同的光源颜色 | 待复核 |
| H2 · 练习 | 练习 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 材质、光源参数、变化光色与原练习 | 设置材质；`02_lighting__03_materials`。03_materials；变化光色在本地练习中提及，需检查是否挤掉正文演示过程。 | 待补齐或复核 |

媒体：4 次上游引用；公式：0 项；提示/译注：0 项；资源与练习链接：2 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 材质 | [materials_real_world.png](../img/02/03/materials_real_world.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 设置材质 | [materials_with_material.png](../img/02/03/materials_with_material.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 光的属性 | [materials_light.png](../img/02/03/materials_light.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 不同的光源颜色 | [materials.mp4](../img/02/03/materials.mp4) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 04 · 光照贴图

上游：[docs/02 Lighting/04 Lighting maps.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/02%20Lighting/04%20Lighting%20maps.md)；本地：[04_lighting_maps.md](04_lighting_maps.md)；源码目标：`02_lighting__04_lighting_maps`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 光照贴图 | 光照贴图 | 待复核 |
| H1 · 漫反射贴图 | 漫反射贴图 | 待复核 |
| H1 · 镜面光贴图 | 镜面贴图 | 待复核 |
| H2 · 采样镜面光贴图 | 采样镜面贴图 | 待复核 |
| H2 · 练习 | 练习 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 漫反射、镜面贴图、彩色镜面与自发光练习 | 练习；`02_lighting__04_lighting_maps`。04_lighting_maps；原练习及其图片仍在，正文深度待验收。 | 待补齐或复核 |

媒体：4 次上游引用；公式：0 项；提示/译注：4 项；资源与练习链接：8 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 漫反射贴图 | [container2.png](../img/02/04/container2.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 漫反射贴图 | [materials_diffuse_map.png](../img/02/04/materials_diffuse_map.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 镜面光贴图 | [container2_specular.png](../img/02/04/container2_specular.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 采样镜面光贴图 | [materials_specular_map.png](../img/02/04/materials_specular_map.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 05 · 投光物

上游：[docs/02 Lighting/05 Light casters.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/02%20Lighting/05%20Light%20casters.md)；本地：[05_light_casters.md](05_light_casters.md)；源码目标：`02_lighting__05_light_casters`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 投光物 | 投光物 | 待复核 |
| H1 · 平行光 | 定向光 | 待复核 |
| H1 · 点光源 | 点光源 | 待复核 |
| H2 · 衰减 | 衰减 | 待复核 |
| H3 · 选择正确的值 | 选择正确的值 | 待复核 |
| H3 · 实现衰减 | 实现衰减 | 待复核 |
| H1 · 聚光 | 聚光灯 | 待复核 |
| H2 · 手电筒 | 手电筒 | 待复核 |
| H2 · 平滑/软化边缘 | 平滑/软化边缘 | 待复核 |
| H2 · 练习 | 练习 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 平行光、点光源、衰减表、手电筒与软边缘 | 聚光灯；`02_lighting__05_light_casters`。05_light_casters 主要运行点光源；06_multiple_lights 包含其他光源，正文每一步的引用与效果待逐项核对。 | 待补齐或复核 |

媒体：9 次上游引用；公式：30 项；提示/译注：3 项；资源与练习链接：0 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 平行光 | [light_casters_directional.png](../img/02/05/light_casters_directional.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 平行光 | [light_casters_directional_light.png](../img/02/05/light_casters_directional_light.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 点光源 | [light_casters_point.png](../img/02/05/light_casters_point.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 衰减 | [attenuation.png](../img/02/05/attenuation.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 实现衰减 | [light_casters_point_light.png](../img/02/05/light_casters_point_light.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 聚光 | [light_casters_spotlight_angles.png](../img/02/05/light_casters_spotlight_angles.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 手电筒 | [light_casters_cos.png](../img/02/05/light_casters_cos.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 手电筒 | [light_casters_spotlight_hard.png](../img/02/05/light_casters_spotlight_hard.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 平滑/软化边缘 | [light_casters_spotlight.png](../img/02/05/light_casters_spotlight.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 06 · 多光源

上游：[docs/02 Lighting/06 Multiple lights.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/02%20Lighting/06%20Multiple%20lights.md)；本地：[06_multiple_lights.md](06_multiple_lights.md)；源码目标：`02_lighting__06_multiple_lights`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 多光源 | 多光源 | 待复核 |
| H2 · 定向光 | 定向光 | 待复核 |
| H2 · 点光源 | 点光源 | 待复核 |
| H2 · 合并结果 | 合并结果 | 待复核 |
| H2 · 练习 | 练习 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 光源函数组合、数组、四点光源、氛围练习 | 合并结果；`02_lighting__06_multiple_lights`。06_multiple_lights；修正六个示例的计数并接回复习篇，内容深度待验收。 | 待补齐或复核 |

媒体：2 次上游引用；公式：0 项；提示/译注：3 项；资源与练习链接：1 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 合并结果 | [multiple_lights_combined.png](../img/02/06/multiple_lights_combined.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 合并结果 | [multiple_lights_atmospheres.png](../img/02/06/multiple_lights_atmospheres.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 07 · 复习

上游：[docs/02 Lighting/07 Review.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/02%20Lighting/07%20Review.md)；本地：[07_review.md](07_review.md)；源码目标：`复习篇，无独立程序`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 复习 | 复习 | **已核验**：本轮补齐；全部 20 个词条保留，附术语修正与规范说明。 |
| H2 · 词汇表 | 词汇表 | **已核验**：本轮补齐；全部 20 个词条保留，附术语修正与规范说明。 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 光照回顾及全部 20 个术语 | 词汇表；`复习篇，无独立程序`。本轮补齐整篇，保留原作者/译者/校对信息并核对英文与 GLSL 规范；无独立程序。 | 已核验 |

媒体：0 次上游引用；公式：0 项；提示/译注：0 项；资源与练习链接：0 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

## 本轮已修正与后续顺序

已补齐原先遗漏的第 07 篇《复习》及全部 20 个术语，修正六个示例的计数，补回环境光效果图，并恢复多光源→复习→Assimp 的阅读链。

前六篇仍需逐段核对中间案例、光照参数、原练习及参考资源；特别检查动态光色是否被移成练习，以及光源类型的程序覆盖是否与每段正文标注一致。

后续修订逐条更新本清单；确认内容及实现两类验收均通过后，才能改写章索引中的完成状态。

## 非标题教学条目盘点

以下是后续逐项核对的定位索引，保留原练习与参考链接；提示摘要仅用于定位，正确解释以本地正文和规范为准。未标记通过的内容仍待核对。

### 01 · 颜色

提示与译注（待逐条核对）：

- 颜色：important。

### 02 · 基础光照

公式与推导（待逐式核对）：漫反射光照（2 项）。

提示与译注（待逐条核对）：

- 漫反射光照：Important。
- 法向量：Important。
- 计算漫反射光照：Important。
- 最后一件事：Attention。
- 镜面光照：Important。
- 镜面光照：Important。

原练习要求：

- 目前，我们的光源是静止的，你可以尝试使用`sin`或`cos`函数让光源在场景中来回移动。观察光照随时间的改变能让你更容易理解风氏光照模型。[参考解答](https://learnopengl.com/code_viewer.php?code=lighting/basic_lighting-exercise1)。
- 尝试使用不同的环境光、漫反射和镜面强度，观察它们怎么是影响光照效果的。同样，尝试实验一下镜面光照的反光度因子。尝试理解为什么某一个值能够有着特定视觉输出。
- 在观察空间（而不是世界空间）中计算风氏光照：[参考解答](https://learnopengl.com/code_viewer.php?code=lighting/basic_lighting-exercise2)。
- 尝试实现一个Gouraud着色（而不是风氏着色）。如果你做对了话，立方体的光照应该会[看起来有些奇怪](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/02/basic_lighting_exercise3.png)，尝试推理为什么它会看起来这么奇怪：[参考解答](https://learnopengl.com/code_viewer.php?code=lighting/basic_lighting-exercise3)。

参考资源与解答链接（待逐项核对）：

- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=lighting/basic_lighting-exercise1)。
- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=lighting/basic_lighting-exercise2)。
- 练习：[看起来有些奇怪](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/02/basic_lighting_exercise3.png)。
- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=lighting/basic_lighting-exercise3)。

### 03 · 材质

原练习要求：

- 你能做到这件事吗，改变光照颜色导致改变光源立方体的颜色？
- 你能像教程一开始那样，通过定义相应的材质来模拟现实世界的物体吗？注意[材质表格](http://devernay.free.fr/cours/opengl/materials.html)中的环境光值与漫反射值不一样，它们没有考虑光照的强度。要想正确地设置它们的值，你需要将所有的光照强度都设置为`vec3(1.0)`，这样才能得到一致的输出：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/3.2.materials_exercise1/materials_exercise1.cpp)：青色塑料(Cyan Plastic)容器。

参考资源与解答链接（待逐项核对）：

- 练习：[材质表格](http://devernay.free.fr/cours/opengl/materials.html)。
- 练习：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/3.2.materials_exercise1/materials_exercise1.cpp)。

### 04 · 光照贴图

提示与译注（待逐条核对）：

- 漫反射贴图：Attention。
- 漫反射贴图：Important。
- 镜面光贴图：important。
- 采样镜面光贴图：important。

原练习要求：

- 调整光源的环境光、漫反射和镜面光向量，看看它们如何影响箱子的视觉输出。
- 尝试在片段着色器中反转镜面光贴图的颜色值，让木头显示镜面高光而钢制边缘不反光（由于钢制边缘中有一些裂缝，边缘仍会显示一些镜面高光，虽然强度会小很多）：[参考解答](https://learnopengl.com/code_viewer.php?code=lighting/lighting_maps-exercise2)
- 使用漫反射贴图创建一个彩色而不是黑白的镜面光贴图，看看结果看起来并不是那么真实了。如果你不会生成的话，可以使用这张[彩色的镜面光贴图](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/lighting_maps_specular_color.png)：[最终效果](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/lighting_maps_exercise3.png)
- 添加一个叫做**放射光贴图**(Emission Map)的东西，它是一个储存了每个片段的发光值(Emission Value)的贴图。发光值是一个包含（假设）光源的物体发光(Emit)时可能显现的颜色，这样的话物体就能够忽略光照条件进行发光(Glow)。游戏中某个物体在发光的时候，你通常看到的就是放射光贴图（比如 [机器人的眼](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/shaders_enemy.jpg)，或是[箱子上的灯带](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/emissive.png)）。将[这个](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/matrix.jpg)纹理（作者为 creativesam）作为放射光贴图添加到箱子上，产生这些字母都在发光的效果：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/4.4.lighting_maps_exercise4/lighting_maps_exercise4.cpp)，[最终效果](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/lighting_maps_exercise4.png)

参考资源与解答链接（待逐项核对）：

- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=lighting/lighting_maps-exercise2)。
- 练习：[彩色的镜面光贴图](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/lighting_maps_specular_color.png)。
- 练习：[最终效果](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/lighting_maps_exercise3.png)。
- 练习：[机器人的眼](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/shaders_enemy.jpg)。
- 练习：[箱子上的灯带](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/emissive.png)。
- 练习：[这个](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/matrix.jpg)。
- 练习：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/2.lighting/4.4.lighting_maps_exercise4/lighting_maps_exercise4.cpp)。
- 练习：[最终效果](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/02/04/lighting_maps_exercise4.png)。

### 05 · 投光物

公式与推导（待逐式核对）：衰减（5 项）；选择正确的值（3 项）；聚光（5 项）；手电筒（2 项）；平滑/软化边缘（15 项）。

提示与译注（待逐条核对）：

- 平行光：Important。
- 实现衰减：Important。
- 手电筒：Important。

原练习要求：

- 尝试实验一下上面的所有光照类型和它们的片段着色器。试着对一些向量进行取反，并使用 < 来代替 >。试着解释不同视觉效果产生的原因。

### 06 · 多光源

提示与译注（待逐条核对）：

- 定向光：Important。
- 点光源：Important。
- 合并结果：Important。

原练习要求：

- 你能通过调节光照属性变量，（大概地）重现最后一张图片上不同的氛围吗？[参考解答](https://learnopengl.com/code_viewer.php?code=lighting/multiple_lights-exercise2)

参考资源与解答链接（待逐项核对）：

- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=lighting/multiple_lights-exercise2)。

### 07 · 复习
