# 第 01 章上游覆盖与核验清单

权威底本：`LearnOpenGL-CN/LearnOpenGL-CN`，分支 `new-theme`，固定提交 `331560886468e28f28326429334fc48470f9c27b`。本次盘点 10 篇、112 个各级标题，并记录非标题案例、公式、提示、练习、媒体与附加资源。以下清单集中记录来源版本和内容去向；教程正文只保留简洁署名。

**验收状态：未通过。** 本清单完成首轮逐项定位与缺口登记，不把“已定位”当成内容等价验收。标题数量、图片散列和源码匹配不能替代逐段教学深度核验；未完成的条目明确保留。

本轮实现验证：MinGW GCC Debug 全项目构建通过；01–03 章仍没有有限帧命令行入口，本轮未重新验证其完整交互与视觉。第 04–06 章已有的运行记录保留在各章覆盖清单，不新增仓库 QA 入口。

## 本次准确性修订

已恢复“矩阵与向量相乘”与“向量 / 矩阵 / 实践”的同级关系；对照英文文章及 OpenGL 3.3 附录 E、GLFW 3.4 和 GLM 1.0.1 修正立即模式/固定管线、弃用/移除、上下文、函数加载、变换后乘顺序、零向量叉乘、相机方向和回调状态说明。

本次移除仓库 QA 入口和重复机器清单；来源版本只在此处集中维护。上述修正不改变其他“缺口 / 待复核”的状态。

## 状态约定

- **已核验**：明确核对过该项内容与对应技术事实；不外推到整章。
- **待复核**：找到对应位置，但尚不能证明概念、推导与案例同等完整。
- **缺口**：确认缺少小节、案例、实现或原教学层级，必须补齐后重新验收。

本仓库新增的全景图、职责说明、进阶块与整体回顾均为补充，不替代底本条目。

## 01 · OpenGL

上游：[docs/01 Getting started/01 OpenGL.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/01%20Getting%20started/01%20OpenGL.md)；本地：[01_opengl.md](01_opengl.md)；源码目标：`概念篇，无独立程序`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · OpenGL | 认识 OpenGL：规范、状态机与对象 | 待复核 |
| H2 · 核心模式与立即渲染模式 | 核心模式与立即渲染模式 | 待复核 |
| H2 · 扩展 | 扩展 | 待复核 |
| H2 · 状态机 | 状态机 | 待复核 |
| H2 · 对象 | 对象 | 待复核 |
| H2 · 让我们开始吧 | 让我们开始吧 | 待复核 |
| H2 · 附加资源 | 附加资源 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 规范、状态、对象与扩展伪代码 | 规范定义 API，驱动提供实现；`概念篇，无独立程序`。已修正概念表述及伪代码标签；其余解释深度待复核。 | 待补齐或复核 |

媒体：1 次上游引用；公式：0 项；提示/译注：4 项；资源与练习链接：2 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| OpenGL | [opengl.jpg](../img/01/01/opengl.jpg) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 02 · 创建窗口

上游：[docs/01 Getting started/02 Creating a window.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/01%20Getting%20started/02%20Creating%20a%20window.md)；本地：[02_creating_a_window.md](02_creating_a_window.md)；源码目标：`概念篇，无独立程序`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 创建窗口 | 创建窗口：准备环境与 OpenGL 上下文 | 待复核 |
| H2 · GLFW | GLFW | 待复核 |
| H2 · 构建GLFW | 构建 GLFW | 待复核 |
| H3 · CMake | CMake | 待复核 |
| H3 · 编译 | 编译 | 待复核 |
| H2 · 我们的第一个工程 | 我们的第一个工程 | 待复核 |
| H2 · 链接 | 链接 | 待复核 |
| H3 · Windows上的OpenGL库 | Windows 上的 OpenGL 库 | 待复核 |
| H3 · Linux上的OpenGL库 | Linux 上的 OpenGL 库 | 待复核 |
| H2 · GLAD | GLAD | 待复核 |
| H3 · 配置GLAD | 配置 GLAD | 待复核 |
| H2 · 附加资源 | 附加资源 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| GLFW 源码构建、链接与 GLAD 加载 | 构建 GLFW；`概念篇，无独立程序`。手动构建与 Conan 路径均存在；版本/链接边界仍需技术复核。 | 待补齐或复核 |

媒体：5 次上游引用；公式：0 项；提示/译注：3 项；资源与练习链接：6 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| CMake | [cmake.png](../img/01/02/cmake.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 我们的第一个工程 | [x64.png](../img/01/02/x64.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 链接 | [vc_directories.png](../img/01/02/vc_directories.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 链接 | [include_directories.png](../img/01/02/include_directories.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 链接 | [linker_input.png](../img/01/02/linker_input.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 03 · 你好，窗口

上游：[docs/01 Getting started/03 Hello Window.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/01%20Getting%20started/03%20Hello%20Window.md)；本地：[03_hello_window.md](03_hello_window.md)；源码目标：`01_getting_started__01_hello_window`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 你好，窗口 | 你好，窗口 | 待复核 |
| H2 · GLAD | 初始化 GLAD | 待复核 |
| H2 · 视口 | 视口 | 待复核 |
| H1 · 准备好你的引擎 | 渲染循环 | 待复核 |
| H2 · 最后一件事 | 清理与退出 | 待复核 |
| H2 · 输入 | 输入：process_input | 待复核 |
| H2 · 渲染 | 渲染：用颜色清空屏幕 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 上下文、视口、输入、清屏、双缓冲 | 渲染循环；`01_getting_started__01_hello_window`。01_hello_window；有限帧入口尚未接入，本轮仅全项目构建。 | 待补齐或复核 |

媒体：2 次上游引用；公式：0 项；提示/译注：5 项；资源与练习链接：0 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 最后一件事 | [hellowindow.png](../img/01/03/hellowindow.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 渲染 | [hellowindow2.png](../img/01/03/hellowindow2.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 04 · 你好，三角形

上游：[docs/01 Getting started/04 Hello Triangle.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/01%20Getting%20started/04%20Hello%20Triangle.md)；本地：[04_hello_triangle.md](04_hello_triangle.md)；源码目标：`01_getting_started__02_hello_triangle`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 你好，三角形 | 你好，三角形 | 待复核 |
| H2 · 顶点输入 | 顶点输入 | 待复核 |
| H2 · 顶点着色器 | 顶点着色器 | 待复核 |
| H2 · 编译着色器 | 编译着色器 | 待复核 |
| H2 · 片段着色器 | 片段着色器 | 待复核 |
| H3 · 着色器程序 | 着色器程序 | 待复核 |
| H2 · 链接顶点属性 | 链接顶点属性 | 待复核 |
| H3 · 顶点数组对象 | 顶点数组对象 | 待复核 |
| H3 · 我们一直期待的三角形 | 我们一直期待的三角形 | 待复核 |
| H2 · 元素缓冲对象 | 元素缓冲对象 | 待复核 |
| H2 · 附加资源 | 附加资源 | 待复核 |
| H1 · 练习 | 练习 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 三角形与索引矩形 | 元素缓冲对象；`01_getting_started__02_hello_triangle`。02_hello_triangle 与 04_textures；补回 EBO 示意图，练习要求保留，仍需逐案例验收。 | 待补齐或复核 |

媒体：7 次上游引用；公式：0 项；提示/译注：15 项；资源与练习链接：7 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 你好，三角形 | [pipeline.png](../img/01/04/pipeline.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 顶点输入 | [ndc.png](../img/01/04/ndc.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 链接顶点属性 | [vertex_attribute_pointer.png](../img/01/04/vertex_attribute_pointer.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 顶点数组对象 | [vertex_array_objects.png](../img/01/04/vertex_array_objects.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 我们一直期待的三角形 | [hellotriangle.png](../img/01/04/hellotriangle.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 元素缓冲对象 | [vertex_array_objects_ebo.png](../img/01/04/vertex_array_objects_ebo.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 元素缓冲对象 | [hellotriangle2.png](../img/01/04/hellotriangle2.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 05 · 着色器

上游：[docs/01 Getting started/05 Shaders.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/01%20Getting%20started/05%20Shaders.md)；本地：[05_shaders.md](05_shaders.md)；源码目标：`01_getting_started__03_shaders`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 着色器 | 着色器 | 待复核 |
| H1 · GLSL | GLSL | 待复核 |
| H2 · 数据类型 | 数据类型 | 待复核 |
| H3 · 向量 | 向量 | 待复核 |
| H2 · 输入与输出 | 输入与输出 | 待复核 |
| H2 · Uniform | Uniform | **缺口**：动态颜色 uniform 主案例只有概念说明与视频，无对应运行目标；exercise2 的水平位移 uniform 不能替代该正文案例。 |
| H2 · 更多属性！ | 更多属性！ | 待复核 |
| H1 · 我们自己的着色器类 | 我们自己的着色器类（仓库做法：匿名命名空间辅助函数） | **缺口**：自由函数适配解释了编译与链接，但删去了文件读取、字符串流、异常/失败处理及完整封装的分步过程。 |
| H2 · 从文件读取 | 无独立对应小节 | **缺口**：独立小节与教学过程缺失；内嵌 GLSL 是工程约定，不能据此删除文件读取的教学内容。 |
| H1 · 练习 | 练习 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 动态 uniform 颜色 | Uniform；`01_getting_started__03_shaders`。待补齐实际案例与分步说明。 | 待补齐或复核 |
| Shader 封装与文件读取 | 我们自己的着色器类（仓库做法：匿名命名空间辅助函数）；`01_getting_started__03_shaders`。待补齐，不能以匿名命名空间的组织差异抵消文件读取教学。 | 待补齐或复核 |
| 颜色插值与三项练习 | 更多属性！；`01_getting_started__03_shaders`。03_shaders 及 exercise1–3；本轮仅构建，交互与视觉待复核。 | 待补齐或复核 |

媒体：4 次上游引用；公式：0 项；提示/译注：4 项；资源与练习链接：3 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 输入与输出 | [shaders.png](../img/01/05/shaders.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| Uniform | [shaders.mp4](../img/01/05/shaders.mp4) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 更多属性！ | [vertex_attribute_pointer_interleaved.png](../img/01/05/vertex_attribute_pointer_interleaved.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 更多属性！ | [shaders3.png](../img/01/05/shaders3.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 06 · 纹理

上游：[docs/01 Getting started/06 Textures.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/01%20Getting%20started/06%20Textures.md)；本地：[06_textures.md](06_textures.md)；源码目标：`01_getting_started__04_textures`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 纹理 | 纹理 | 待复核 |
| H2 · 纹理环绕方式 | 纹理环绕方式 | 待复核 |
| H2 · 纹理过滤 | 纹理过滤 | 待复核 |
| H3 · 多级渐远纹理 | 多级渐远纹理 | 待复核 |
| H1 · 加载与创建纹理 | 加载与创建纹理 | 待复核 |
| H2 · stb_image.h | stb_image.h | 待复核 |
| H2 · 生成纹理 | 生成纹理 | 待复核 |
| H2 · 应用纹理 | 应用纹理 | 待复核 |
| H2 · 纹理单元 | 纹理单元 | 待复核 |
| H2 · 练习 | 练习 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 环绕、过滤、mipmap 与双纹理混合 | 纹理单元；`01_getting_started__04_textures`。04_textures；正文/资源定位完成，案例过程与全部练习待逐段复核。 | 待补齐或复核 |

媒体：12 次上游引用；公式：0 项；提示/译注：5 项；资源与练习链接：5 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 纹理 | [textures.png](../img/01/06/textures.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 纹理 | [tex_coords.png](../img/01/06/tex_coords.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 纹理环绕方式 | [texture_wrapping.png](../img/01/06/texture_wrapping.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 纹理过滤 | [filter_nearest.png](../img/01/06/filter_nearest.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 纹理过滤 | [filter_linear.png](../img/01/06/filter_linear.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 纹理过滤 | [texture_filtering.png](../img/01/06/texture_filtering.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 多级渐远纹理 | [mipmaps.png](../img/01/06/mipmaps.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 应用纹理 | [vertex_attribute_pointer_interleaved_textures.png](../img/01/06/vertex_attribute_pointer_interleaved_textures.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 应用纹理 | [textures2.png](../img/01/06/textures2.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 应用纹理 | [textures_funky.png](../img/01/06/textures_funky.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 纹理单元 | [textures_combined.png](../img/01/06/textures_combined.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 纹理单元 | [textures_combined2.png](../img/01/06/textures_combined2.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 07 · 变换

上游：[docs/01 Getting started/07 Transformations.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/01%20Getting%20started/07%20Transformations.md)；本地：[07_transformations.md](07_transformations.md)；源码目标：`01_getting_started__05_transformations`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 变换 | 变换 | 待复核 |
| H1 · 向量 | 向量 | 待复核 |
| H2 · 向量与标量运算 | 向量与标量运算 | 待复核 |
| H2 · 向量取反 | 向量取反 | 待复核 |
| H2 · 向量加减 | 向量加减 | 待复核 |
| H2 · 长度 | 长度 | 待复核 |
| H2 · 向量相乘 | 向量相乘 | 待复核 |
| H3 · 点乘 | 点乘 | 待复核 |
| H3 · 叉乘 | 叉乘 | 待复核 |
| H1 · 矩阵 | 矩阵 | 待复核 |
| H2 · 矩阵的加减 | 矩阵的加减 | 待复核 |
| H2 · 矩阵的数乘 | 矩阵的数乘 | 待复核 |
| H2 · 矩阵相乘 | 矩阵相乘 | 待复核 |
| H1 · 矩阵与向量相乘 | 矩阵与向量相乘 | 待复核 |
| H2 · 单位矩阵 | 单位矩阵 | 待复核 |
| H2 · 缩放 | 缩放 | 待复核 |
| H2 · 位移 | 位移 | 待复核 |
| H2 · 旋转 | 旋转 | 待复核 |
| H2 · 矩阵的组合 | 矩阵的组合 | 待复核 |
| H1 · 实践 | 实践 | 待复核 |
| H2 · GLM | 实践 | **缺口**：GLM 内容存在于“实践”，独立小节缺失；上游固定旋转/缩放与动画组合需逐案例重新对应，而非只引用最终动画程序。 |
| H2 · 拓展阅读 | 拓展阅读 | 待复核 |
| H2 · 练习 | 练习 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 向量、矩阵、轴角、矩阵复合、GLM 与动画 | 实践；`01_getting_started__05_transformations`。05_transformations；公式目录已盘点；默认初始化、万向节死锁表述已纠正，固定变换/组合案例待补齐核验。 | 待补齐或复核 |

媒体：11 次上游引用；公式：56 项；提示/译注：9 项；资源与练习链接：4 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 向量 | [vectors.png](../img/01/07/vectors.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 向量加减 | [vectors_addition.png](../img/01/07/vectors_addition.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 向量加减 | [vectors_subtraction.png](../img/01/07/vectors_subtraction.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 长度 | [vectors_triangle.png](../img/01/07/vectors_triangle.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 叉乘 | [vectors_crossproduct.png](../img/01/07/vectors_crossproduct.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 矩阵相乘 | [matrix_multiplication.png](../img/01/07/matrix_multiplication.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 缩放 | [vectors_scale.png](../img/01/07/vectors_scale.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 旋转 | [vectors_angle.png](../img/01/07/vectors_angle.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| GLM | [glm.png](../img/01/07/glm.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| GLM | [transformations.png](../img/01/07/transformations.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| GLM | [transformations.mp4](../img/01/07/transformations.mp4) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 08 · 坐标系统

上游：[docs/01 Getting started/08 Coordinate Systems.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/01%20Getting%20started/08%20Coordinate%20Systems.md)；本地：[08_coordinate_systems.md](08_coordinate_systems.md)；源码目标：`01_getting_started__06_coordinate_systems`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 坐标系统 | 坐标系统 | 待复核 |
| H2 · 概述 | 概述 | 待复核 |
| H2 · 局部空间 | 局部空间 | 待复核 |
| H2 · 世界空间 | 世界空间 | 待复核 |
| H2 · 观察空间 | 观察空间 | 待复核 |
| H2 · 裁剪空间 | 裁剪空间 | 待复核 |
| H3 · 正射投影 | 正射投影 | 待复核 |
| H3 · 透视投影 | 透视投影 | 待复核 |
| H2 · 把它们都组合到一起 | 把它们都组合到一起 | 待复核 |
| H1 · 进入3D | 进入 3D | 待复核 |
| H2 · 更加 3D | 进入 3D | **缺口**：本地合并进“进入 3D”，从倾斜平面到单立方体再到多立方体的教学层级与案例过程未按底本完整保留。 |
| H3 · Z缓冲 | 深度测试：让遮挡正确 | 待复核 |
| H3 · 更多的立方体！ | 渲染 10 个立方体 | 待复核 |
| H2 · 练习 | 练习 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 倾斜平面→单立方体→深度测试→十立方体 | 进入 3D；`01_getting_started__06_coordinate_systems`。06_coordinate_systems 直接运行多立方体；中间案例待补齐，原平面结果图已恢复。 | 待补齐或复核 |

媒体：10 次上游引用；公式：2 项；提示/译注：4 项；资源与练习链接：1 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 概述 | [coordinate_systems.png](../img/01/08/coordinate_systems.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 正射投影 | [orthographic_frustum.png](../img/01/08/orthographic_frustum.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 透视投影 | [perspective.png](../img/01/08/perspective.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 透视投影 | [perspective_frustum.png](../img/01/08/perspective_frustum.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 透视投影 | [perspective_orthographic.png](../img/01/08/perspective_orthographic.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 进入3D | [coordinate_systems_right_handed.png](../img/01/08/coordinate_systems_right_handed.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 进入3D | [coordinate_systems_result.png](../img/01/08/coordinate_systems_result.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 更加 3D | [coordinate_system_no_depth.mp4](../img/01/08/coordinate_system_no_depth.mp4) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| Z缓冲 | [coordinate_system_depth.mp4](../img/01/08/coordinate_system_depth.mp4) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 更多的立方体！ | [coordinate_systems_multiple_objects.png](../img/01/08/coordinate_systems_multiple_objects.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 09 · 摄像机

上游：[docs/01 Getting started/09 Camera.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/01%20Getting%20started/09%20Camera.md)；本地：[09_camera.md](09_camera.md)；源码目标：`01_getting_started__07_camera`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 摄像机 | 摄像机 | 待复核 |
| H2 · 摄像机/观察空间 | 摄像机/观察空间 | 待复核 |
| H3 · 1. 摄像机位置 | 1. 摄像机位置 | 待复核 |
| H3 · 2. 摄像机方向 | 2. 摄像机方向 | 待复核 |
| H3 · 3. 右轴 | 3. 右轴 | 待复核 |
| H3 · 4. 上轴 | 4. 上轴 | 待复核 |
| H2 · Look At | Look At | **缺口**：环绕摄像机被缩为一段解释与视频链接，缺少与自由移动同等展开的分步案例及实际运行入口。 |
| H1 · 自由移动 | 自由移动 | 待复核 |
| H2 · 移动速度 | 移动速度与 delta_time | 待复核 |
| H1 · 视角移动 | 视角移动 | 待复核 |
| H2 · 欧拉角 | 欧拉角 | 待复核 |
| H2 · 鼠标输入 | 鼠标输入 | 待复核 |
| H2 · 缩放 | 缩放 | 待复核 |
| H1 · 摄像机类 | 摄像机类的取舍 | 待复核 |
| H2 · 练习 | 练习 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 环绕、自由移动、时间步、欧拉角与输入 | Look At；`01_getting_started__07_camera`。07_camera；方向轴约定已纠正，环绕模式和完整教学过程待补齐。 | 待补齐或复核 |
| 相机封装、局限与练习 | 摄像机类的取舍；`01_getting_started__07_camera`。匿名命名空间保存状态的选择存在；仍需逐项检查原文内容，不能把文件内全局状态误称唯一方案。 | 待补齐或复核 |

媒体：8 次上游引用；公式：8 项；提示/译注：5 项；资源与练习链接：2 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

| 上游媒体所在小节 | 本地资源 | 状态 |
|---|---|---|
| 摄像机/观察空间 | [camera_axes.png](../img/01/09/camera_axes.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| Look At | [camera_circle.mp4](../img/01/09/camera_circle.mp4) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 移动速度 | [camera_smooth.mp4](../img/01/09/camera_smooth.mp4) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 欧拉角 | [camera_pitch_yaw_roll.png](../img/01/09/camera_pitch_yaw_roll.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 欧拉角 | [camera_triangle.png](../img/01/09/camera_triangle.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 欧拉角 | [camera_pitch.png](../img/01/09/camera_pitch.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 欧拉角 | [camera_yaw.png](../img/01/09/camera_yaw.png) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |
| 缩放 | [camera_mouse.mp4](../img/01/09/camera_mouse.mp4) | 已引用；字节散列另由检查入口核对，图文作用与顺序待验收 |

## 10 · 复习

上游：[docs/01 Getting started/10 Review.md](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/01%20Getting%20started/10%20Review.md)；本地：[10_review.md](10_review.md)；源码目标：`概念篇，无独立程序`。

| 上游小节（保留原层级） | 本地小节 | 状态与说明 |
|---|---|---|
| H1 · 复习 | 复习 | 待复核 |
| H2 · 词汇表 | 词汇表 | 待复核 |

| 非标题教学案例 | 对应位置与源码/效果 | 状态 |
|---|---|---|
| 入门回顾与 26 个术语 | 词汇表；`概念篇，无独立程序`。词条定位完成；修正 NDC、VBO 与 uniform 表述，剩余定义待逐项技术验收。 | 待补齐或复核 |

媒体：0 次上游引用；公式：0 项；提示/译注：0 项；资源与练习链接：0 项。这些数量是盘点索引，不是验收结论。资源逐项去向见下表，其他教学条目见文末盘点。

## 本轮已修正与后续顺序

已纠正 OpenGL API/规范措辞、相机前后方向与叉乘约定、GLM 默认初始化说明，规范化变换与坐标系的单 H1；独立概念片段已明确标记，补回三角形 EBO、uniform 动画和平面透视的媒体引用。

优先补齐 Shader 的动态 uniform 和文件读取教学，再恢复变换/坐标系中间案例及摄像机环绕案例。所有公式、译注和练习仍须逐项阅读核对；旧版 `\(...\)` 公式界定符已转换为 GFM 的 `$...$`，不据此声称公式本身已全部正确。

后续修订逐条更新本清单；确认内容及实现两类验收均通过后，才能改写章索引中的完成状态。

## 非标题教学条目盘点

以下是后续逐项核对的定位索引，保留原练习与参考链接；提示摘要仅用于定位，正确解释以本地正文和规范为准。未标记通过的内容仍待核对。

### 01 · OpenGL

提示与译注（待逐条核对）：

- OpenGL：important。
- 核心模式与立即渲染模式：attention。
- 对象：note "译注"。
- 对象：attention。

参考资源与解答链接（待逐项核对）：

- 附加资源：[opengl.org](https://www.opengl.org/)。
- 附加资源：[OpenGL registry](https://www.opengl.org/registry/)。

### 02 · 创建窗口

提示与译注（待逐条核对）：

- 创建窗口：note "译注"。
- 构建GLFW：Attention。
- Linux上的OpenGL库：Important。

参考资源与解答链接（待逐项核对）：

- 附加资源：[GLFW: Window Guide](http://www.glfw.org/docs/latest/window_guide.html)。
- 附加资源：[Building applications](http://www.opengl-tutorial.org/miscellaneous/building-your-own-c-application/)。
- 附加资源：[GLFW with Code::Blocks](http://wiki.codeblocks.org/index.php?title=Using_GLFW_with_Code::Blocks)。
- 附加资源：[Running CMake](http://www.cmake.org/runningcmake/)。
- 附加资源：[Writing a build system under Linux](http://learnopengl.com/demo/autotools_tutorial.txt)。
- 附加资源：[Polytonic/Glitter](https://github.com/Polytonic/Glitter)。

### 03 · 你好，窗口

提示与译注（待逐条核对）：

- 你好，窗口：Attention。
- 你好，窗口：Important。
- 视口：Important。
- 准备好你的引擎：Important。
- 渲染：Important。

### 04 · 你好，三角形

提示与译注（待逐条核对）：

- 你好，三角形：note "译注"。
- 你好，三角形：Important。
- 你好，三角形：note "译注1"。
- 你好，三角形：Important。
- 你好，三角形：Important。
- 顶点输入：note "译注2"。
- 顶点输入：Important。
- 顶点着色器：Important。
- 编译着色器：Important。
- 片段着色器：Important。
- 着色器程序：Important。
- 链接顶点属性：Important。
- 顶点数组对象：Attention。
- 元素缓冲对象：Attention。
- 元素缓冲对象：Important。

原练习要求：

- 添加更多顶点到数据中，使用`glDrawArrays`，尝试绘制两个彼此相连的三角形：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/2.3.hello_triangle_exercise1/hello_triangle_exercise1.cpp)
- 创建相同的两个三角形，但对它们的数据使用不同的VAO和VBO：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/2.4.hello_triangle_exercise2/hello_triangle_exercise2.cpp)
- 创建两个着色器程序，第二个程序使用一个不同的片段着色器，输出黄色；再次绘制这两个三角形，让其中一个输出为黄色：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/2.5.hello_triangle_exercise3/hello_triangle_exercise3.cpp)

参考资源与解答链接（待逐项核对）：

- 附加资源：[antongerdelan.net/hellotriangle](http://antongerdelan.net/opengl/hellotriangle.html)。
- 附加资源：[open.gl/drawing](https://open.gl/drawing)。
- 附加资源：[antongerdelan.net/vertexbuffers](http://antongerdelan.net/opengl/vertexbuffers.html)。
- 附加资源：[调试](https://learnopengl.com/#!In-Practice/Debugging)。
- 练习：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/2.3.hello_triangle_exercise1/hello_triangle_exercise1.cpp)。
- 练习：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/2.4.hello_triangle_exercise2/hello_triangle_exercise2.cpp)。
- 练习：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/2.5.hello_triangle_exercise3/hello_triangle_exercise3.cpp)。

### 05 · 着色器

提示与译注（待逐条核对）：

- 输入与输出：Important。
- Uniform：Attention。
- Uniform：Important。
- 我们自己的着色器类：Important。

原练习要求：

- 修改顶点着色器让三角形上下颠倒：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/shaders-exercise1)
- 使用uniform定义一个水平偏移量，在顶点着色器中使用这个偏移量把三角形移动到屏幕右侧：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/shaders-exercise2)
- 使用`out`关键字把顶点位置输出到片段着色器，并将片段的颜色设置为与顶点位置相等（来看看连顶点位置值都在三角形中被插值的结果）。做完这些后，尝试回答下面的问题：为什么在三角形的左下角是黑的?：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/shaders-exercise3)

参考资源与解答链接（待逐项核对）：

- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/shaders-exercise1)。
- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/shaders-exercise2)。
- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/shaders-exercise3)。

### 06 · 纹理

提示与译注（待逐条核对）：

- 纹理：note "译注"。
- 纹理：Important。
- 纹理过滤：note "译注1"。
- 应用纹理：attention。
- 纹理单元：Important。

原练习要求：

- 修改片段着色器，**仅**让笑脸图案朝另一个方向看，[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/4.3.textures_exercise1/textures_exercise1.cpp)
- 尝试用不同的纹理环绕方式，设定一个从`0.0f`到`2.0f`范围内的（而不是原来的`0.0f`到`1.0f`）纹理坐标。试试看能不能在箱子的角落放置4个笑脸：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/4.4.textures_exercise2/textures_exercise2.cpp)，[结果](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/01/06/textures_exercise2.png)。记得一定要试试其它的环绕方式。
- 尝试在矩形上只显示纹理图像的中间一部分，修改纹理坐标，达到能看见单个的像素的效果。尝试使用`GL_NEAREST`的纹理过滤方式让像素显示得更清晰：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/4.5.textures_exercise3/textures_exercise3.cpp)
- 使用一个uniform变量作为`mix`函数的第三个参数来改变两个纹理可见度，使用上和下键来改变箱子或笑脸的可见度：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/4.6.textures_exercise4/textures_exercise4.cpp)。

参考资源与解答链接（待逐项核对）：

- 练习：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/4.3.textures_exercise1/textures_exercise1.cpp)。
- 练习：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/4.4.textures_exercise2/textures_exercise2.cpp)。
- 练习：[结果](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN/blob/331560886468e28f28326429334fc48470f9c27b/docs/img/01/06/textures_exercise2.png)。
- 练习：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/4.5.textures_exercise3/textures_exercise3.cpp)。
- 练习：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/4.6.textures_exercise4/textures_exercise4.cpp)。

### 07 · 变换

公式与推导（待逐式核对）：向量（4 项）；向量与标量运算（1 项）；向量取反（1 项）；向量加减（2 项）；长度（8 项）；向量相乘（2 项）；点乘（9 项）；叉乘（1 项）；矩阵（1 项）；矩阵的加减（4 项）；矩阵的数乘（1 项）；矩阵相乘（3 项）；单位矩阵（2 项）；缩放（5 项）；位移（2 项）；旋转（8 项）；矩阵的组合（2 项）。

提示与译注（待逐条核对）：

- 向量与标量运算：note "译注"。
- 点乘：Important。
- 矩阵的加减：note "译注"。
- 单位矩阵：Important。
- 位移：Important。
- 旋转：Important。
- 旋转：note "译注"。
- GLM：attention。
- GLM：Attention。

原练习要求：

- 使用应用在箱子上的最后一个变换，尝试将其改变为先旋转，后位移。看看发生了什么，试着想想为什么会发生这样的事情：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/transformations-exercise1)
- 尝试再次调用`glDrawElements`画出第二个箱子，**只**使用变换将其摆放在不同的位置。让这个箱子被摆放在窗口的左上角，并且会不断的缩放（而不是旋转）。（`sin`函数在这里会很有用，不过注意使用`sin`函数时应用负值会导致物体被翻转）：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/5.2.transformations_exercise2/transformations_exercise2.cpp)

参考资源与解答链接（待逐项核对）：

- 拓展阅读：[线性代数的本质](https://www.youtube.com/playlist?list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab)。
- 拓展阅读：[中文字幕版本](http://space.bilibili.com/88461692#!/channel/detail?cid=9450)。
- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/transformations-exercise1)。
- 练习：[参考解答](https://learnopengl.com/code_viewer_gh.php?code=src/1.getting_started/5.2.transformations_exercise2/transformations_exercise2.cpp)。

### 08 · 坐标系统

公式与推导（待逐式核对）：透视投影（1 项）；把它们都组合到一起（1 项）。

提示与译注（待逐条核对）：

- 裁剪空间：important。
- 透视投影：important。
- 把它们都组合到一起：important。
- 进入3D：important。

原练习要求：

- 对GLM的`projection`函数中的`FoV`和`aspect-ratio`参数进行实验。看能否搞懂它们是如何影响透视平截头体的。
- 将观察矩阵在各个方向上进行位移，来看看场景是如何改变的。注意把观察矩阵当成摄像机对象。
- 使用模型矩阵只让是3倍数的箱子旋转（以及第1个箱子），而让剩下的箱子保持静止。[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/coordinate_systems-exercise3)。

参考资源与解答链接（待逐项核对）：

- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/coordinate_systems-exercise3)。

### 09 · 摄像机

公式与推导（待逐式核对）：Look At（5 项）；欧拉角（3 项）。

提示与译注（待逐条核对）：

- 1. 摄像机位置：important。
- 2. 摄像机方向：Attention。
- 自由移动：important。
- 缩放：Important。
- 摄像机类：Attention。

原练习要求：

- 看看你是否能够修改摄像机类，使得其能够变成一个**真正的**FPS摄像机（也就是说不能够随意飞行）；你只能够呆在xz平面上：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/camera-exercise1)
- 试着创建你自己的LookAt函数，其中你需要手动创建一个我们在一开始讨论的观察矩阵。用你的函数实现来替换GLM的LookAt函数，看看它是否还能一样地工作：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/camera-exercise2)

参考资源与解答链接（待逐项核对）：

- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/camera-exercise1)。
- 练习：[参考解答](https://learnopengl.com/code_viewer.php?code=getting-started/camera-exercise2)。

### 10 · 复习
