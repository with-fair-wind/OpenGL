# 第 04 章上游覆盖与验证记录

中文底本：[LearnOpenGL-CN/LearnOpenGL-CN](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN)，分支 `new-theme`，固定提交 `331560886468e28f28326429334fc48470f9c27b`。本次按该版本的 11 篇完整文章盘点，不以 fork 或浮动分支作为验收基准。正文依原教学顺序重写，示例代码适配本仓库 C++23 实现；多 H1 文章规范化为单 H1，其余标题保持对应内容和父子关系。

## 验收范围

- 内容：逐节核对概念、公式/推导、参数与状态含义、案例过程、注意事项、原有练习和资源；下面列出全部标题映射，并另列非标题条目。
- 实现：11 个独立目标、48 个启动模式。MinGW GCC Debug configure/build 通过，NVIDIA GeForce GTX 1660 上各模式运行 3 帧、保存 PPM、转换 PNG 并检查合成图；运行时包含 shader/链接、资源加载、FBO 和 GL 错误检查。
- 格式：C++/GLSL 代码块均与各篇指定 `main.cpp` 连续文本逐字匹配；本地文件/图片/锚点、GFM、全景图、回顾和下一节链另外校验。
- 未验证范围：其他 preset/显卡的实际运行、完整键鼠/缩放交互、性能基准；不以本次功能检查声称任何固定 FPS 或跨平台性能收益。
- 原有练习仍标为练习：反向环绕顶点、后视镜、反射遮罩和自选滤镜。动态环境捕获和保守深度等原文概念按原深度解释；动态捕获不冒称已实现，4.2 功能不冒充 3.3 核心功能。

## 2026-10-22 复核问题修正

以下记录针对本次复核发现的五项问题，不用有限帧成功替代上游内容核验。

| 问题 | 修正与验证 |
| --- | --- |
| 十万实例栈溢出 | `10_instancing` 改用堆上的 vector；上传长度按元素数乘矩阵大小计算。隔离源码副本只将 amount 从 1000 改为 100000，模式 3 运行 3 帧成功，无 stderr。 |
| 必需 uniform 缺失仍返回成功 | 09/10/11 的相关查询移至初始化并检查，渲染时使用缓存。分别将 time_seconds、use_instances、sample_count 的 CPU 查询名称改错，三个隔离副本均报告缺失并返回非零。 |
| 共面深度冲突案例缺失 | 01 增加模式 4/5、箱内预置视角、相同投影下的共面/分离几何及实测截图；恢复原案例的复现步骤。 |
| 独立物体轮廓提示遗漏 | 02 恢复逐物体清模板与深度处理的原提示，并补充选中标记、模糊轮廓、后视镜和阴影体积的应用说明；这些扩展不冒称已运行实现。 |
| 混合图文数值不一致 | 03 改为与原图一致的绿色 alpha=0.6 覆盖红色，逐项写出贡献及结果 (0.4,0.6,0)。 |

修正后 11 个目标、48 个模式均在 MinGW GCC Debug / GTX 1660 上运行 3 帧并保存末帧；新增共面/分离截图另行目视核对。故障注入和十万实例检查只修改隔离副本，仓库默认实例数仍为 1000，不据此声称十万实例的性能基准。

## 逐小节对应

“源码/模式”编号的完整可观察行为见各篇模式表；同一目标的二维与三维模式明确分开。模式只验证它实际实现的路径，不替代内容检查。


### 01 深度测试

底本文件：`docs/04 Advanced OpenGL/01 Depth testing.md`。默认深度、8 种比较、投影深度推导与逆变换、三类深度冲突处理；修正定点/浮点和坐标空间术语。

| 上游标题（原级别） | 本地对应 | 源码/模式与内容项 | 图示/公式 | 状态 |
| --- | --- | --- | --- | --- |
| # 深度测试 | [深度测试](01_depth_testing.md#深度测试)（H1） | `01_depth_testing`：0–5 | `depth_testing_func_always.png`、`depth_testing_func_less.png`、`depth_linear_graph.png`、`depth_non_linear_graph.png`、`depth_testing_visible_depth.png`、`depth_testing_visible_linear.png`、`depth_testing_z_fighting.png`、`01_depth_testing.png`；公式：展示3组/行内27处 | 已覆盖；相关模式通过 |
| ## 深度测试函数 | [深度测试函数](01_depth_testing.md#深度测试函数)（H2） | `01_depth_testing`：0 / 3；其他比较见表 | `depth_testing_func_always.png`、`depth_testing_func_less.png`；公式：展示0组/行内8处 | 已覆盖；相关模式通过 |
| ## 深度值精度 | [深度值精度](01_depth_testing.md#深度值精度)（H2） | `01_depth_testing`：1 / 2；投影推导 | `depth_linear_graph.png`、`depth_non_linear_graph.png`；公式：展示2组/行内12处 | 已覆盖（概念） |
| ## 深度缓冲的可视化 | [深度缓冲的可视化](01_depth_testing.md#深度缓冲的可视化)（H2） | `01_depth_testing`：1 / 2 | `depth_testing_visible_depth.png`、`depth_testing_visible_linear.png`；公式：展示1组/行内6处 | 已覆盖；相关模式通过 |
| ## 深度冲突 | [深度冲突](01_depth_testing.md#深度冲突)（H2） | `01_depth_testing`：4；箱内视角、共面高度与复现步骤 | `depth_testing_z_fighting.png`、`depth_conflict.png` | 已补齐；相关模式通过 |
| ### 防止深度冲突 | [防止深度冲突](01_depth_testing.md#防止深度冲突)（H3） | `01_depth_testing`：5；抬高箱子消除共面，另保留近裁剪距离与深度格式说明 | `depth_separated.png`、几何高度源码 | 已补齐；几何对照模式通过 |

图片/公式：`depth_testing_func_always.png`、`depth_testing_func_less.png`、`depth_linear_graph.png`、`depth_non_linear_graph.png`、`depth_testing_visible_depth.png`、`depth_testing_visible_linear.png`、`depth_testing_z_fighting.png`

非标题内容项：默认深度、8 种比较、投影深度推导与逆变换、三类深度冲突处理；修正定点/浮点和坐标空间术语。 实际摘录与截图见正文；练习保留为原练习，正文主案例未降为练习。


### 02 模板测试

底本文件：`docs/04 Advanced OpenGL/02 Stencil testing.md`。比较/写掩码、8 种模板操作、双物体轮廓 8 步与状态恢复；零写掩码保留值而非写零。

| 上游标题（原级别） | 本地对应 | 源码/模式与内容项 | 图示/公式 | 状态 |
| --- | --- | --- | --- | --- |
| # 模板测试 | [模板测试](02_stencil_testing.md#模板测试)（H1） | `02_stencil_testing`：0 | `stencil_buffer.png`、`stencil_object_outlining.png`、`stencil_scene_outlined.png`、`02_stencil_testing.png`；公式：展示0组/行内9处 | 已覆盖；相关模式通过 |
| ## 模板函数 | [模板函数](02_stencil_testing.md#模板函数)（H2） | `02_stencil_testing`：0；比较/操作完整表 | ；公式：展示0组/行内9处 | 已覆盖；相关模式通过 |
| # 物体轮廓 | [物体轮廓](02_stencil_testing.md#物体轮廓)（H2） | `02_stencil_testing`：0；双物体八步；另保留逐物体清模板与深度配合提示 | `stencil_object_outlining.png`、`stencil_scene_outlined.png` | 已覆盖；相关模式通过 |

图片/公式：`stencil_buffer.png`、`stencil_object_outlining.png`、`stencil_scene_outlined.png`

非标题内容项：比较/写掩码、8 种模板操作、双物体轮廓 8 步与状态恢复；零写掩码保留值而非写零。 实际摘录与截图见正文；练习保留为原练习，正文主案例未降为练习。


### 03 混合

底本文件：`docs/04 Advanced OpenGL/03 Blending.md`。RGBA 草/玻璃加载、discard、混合因子/方程、排序限制；保留边缘采样译注，使用稳定排序防止等距离丢对象。

| 上游标题（原级别） | 本地对应 | 源码/模式与内容项 | 图示/公式 | 状态 |
| --- | --- | --- | --- | --- |
| # 混合 | [混合](03_blending.md#混合)（H1） | `03_blending`：0–3 | `blending_transparency.png`、`blending_transparent_window.png`、`grass.png`、`blending_no_discard.png`、`blending_discard.png`、`blending_equation.png`、`blending_equation_mixed.png`、`blending_incorrect_order.png`、`blending_sorted.png`、`03_blending.png`；公式：展示1组/行内25处 | 已覆盖；相关模式通过 |
| ## 丢弃片段 | [丢弃片段](03_blending.md#丢弃片段)（H2） | `03_blending`：2 / 3；RGBA 加载 | `grass.png`、`blending_no_discard.png`、`blending_discard.png` | 已覆盖；相关模式通过 |
| ## 混合 | [混合](03_blending.md#混合-1)（H2） | `03_blending`：0 / 1；因子与方程表 | `blending_equation.png`、`blending_equation_mixed.png`；公式：展示1组/行内24处 | 已覆盖；相关模式通过 |
| ## 渲染半透明纹理 | [渲染半透明纹理](03_blending.md#渲染半透明纹理)（H2） | `03_blending`：1；未排序对照 | `blending_incorrect_order.png` | 已覆盖；相关模式通过 |
| ## 不要打乱顺序 | [不要打乱顺序](03_blending.md#不要打乱顺序)（H2） | `03_blending`：0 / 1；排序与深度写入 | `blending_sorted.png`；公式：展示0组/行内1处 | 已覆盖；相关模式通过 |

图片/公式：`blending_transparency.png`、`blending_transparent_window.png`、`grass.png`、`blending_no_discard.png`、`blending_discard.png`、`blending_equation.png`、`blending_equation_mixed.png`、`blending_incorrect_order.png`、`blending_sorted.png`

非标题内容项：RGBA 草/玻璃加载、discard、混合因子/方程、排序限制；保留边缘采样译注，使用稳定排序防止等距离丢对象。 实际摘录与截图见正文；练习保留为原练习，正文主案例未降为练习。


### 04 面剔除

底本文件：`docs/04 Advanced OpenGL/04 Face culling.md`。环绕数据、三种剔除目标、CW/CCW 与原反向顶点练习；删除无条件 50% 性能保证。

| 上游标题（原级别） | 本地对应 | 源码/模式与内容项 | 图示/公式 | 状态 |
| --- | --- | --- | --- | --- |
| # 面剔除 | [面剔除](04_face_culling.md#面剔除)（H1） | `04_face_culling`：0 / 1 | `faceculling_windingorder.png`、`faceculling_frontback.png`、`faceculling_reverse.png`、`04_face_culling.png` | 已覆盖；相关模式通过 |
| ## 环绕顺序 | [环绕顺序](04_face_culling.md#环绕顺序)（H2） | `04_face_culling`：0 / 1；窗口空间定向 | `faceculling_windingorder.png`、`faceculling_frontback.png` | 已覆盖；相关模式通过 |
| ## 面剔除 | [面剔除](04_face_culling.md#面剔除-1)（H2） | `04_face_culling`：0 / 1；三种剔除目标表 | `faceculling_reverse.png` | 已覆盖；相关模式通过 |
| ## 练习 | [练习](04_face_culling.md#练习)（H2） | `04_face_culling`：原顶点反向练习，未冒称已实现 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已保留原练习 |

图片/公式：`faceculling_windingorder.png`、`faceculling_frontback.png`、`faceculling_reverse.png`

非标题内容项：环绕数据、三种剔除目标、CW/CCW 与原反向顶点练习；删除无条件 50% 性能保证。 实际摘录与截图见正文；练习保留为原练习，正文主案例未降为练习。


### 05 帧缓冲

底本文件：`docs/04 Advanced OpenGL/05 Framebuffers.md`。FBO/RBO/纹理完整链路、两种灰度、三个卷积案例、边界译注、两项练习；修正 RBO 不可读及核权重和表述。

| 上游标题（原级别） | 本地对应 | 源码/模式与内容项 | 图示/公式 | 状态 |
| --- | --- | --- | --- | --- |
| # 帧缓冲 | [帧缓冲](05_framebuffers.md#帧缓冲)（H1） | `05_framebuffers`：0–6 | `framebuffers_screen_texture.png`、`framebuffers_inverse.png`、`framebuffers_grayscale.png`、`framebuffers_sharpen.png`、`framebuffers_blur.png`、`framebuffers_edge_detection.png`、`05_framebuffers.png`；公式：展示3组/行内6处 | 已覆盖；相关模式通过 |
| ## 创建一个帧缓冲 | [创建一个帧缓冲](05_framebuffers.md#创建一个帧缓冲)（H2） | `05_framebuffers`：所有模式的 resize_target | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖；相关模式通过 |
| ### 纹理附件 | [纹理附件](05_framebuffers.md#纹理附件)（H3） | `05_framebuffers`：颜色纹理与深度纹理概念 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖（概念） |
| ### 渲染缓冲对象附件 | [渲染缓冲对象附件](05_framebuffers.md#渲染缓冲对象附件)（H3） | `05_framebuffers`：组合深度/模板 RBO | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖；相关模式通过 |
| ## 渲染到纹理 | [渲染到纹理](05_framebuffers.md#渲染到纹理)（H2） | `05_framebuffers`：0；两遍渲染 | `framebuffers_screen_texture.png`；公式：展示0组/行内1处 | 已覆盖；相关模式通过 |
| # 后期处理 | [后期处理](05_framebuffers.md#后期处理)（H2） | `05_framebuffers`：1–6 | `framebuffers_inverse.png`、`framebuffers_grayscale.png`、`framebuffers_sharpen.png`、`framebuffers_blur.png`、`framebuffers_edge_detection.png`；公式：展示3组/行内5处 | 已覆盖；相关模式通过 |
| ### 反相 | [反相](05_framebuffers.md#反相)（H3） | `05_framebuffers`：1 | `framebuffers_inverse.png`；公式：展示0组/行内1处 | 已覆盖；相关模式通过 |
| ### 灰度 | [灰度](05_framebuffers.md#灰度)（H3） | `05_framebuffers`：2 / 6 | `framebuffers_grayscale.png`；公式：展示0组/行内2处 | 已覆盖；相关模式通过 |
| ## 核效果 | [核效果](05_framebuffers.md#核效果)（H3） | `05_framebuffers`：3；核的公式与循环 | `framebuffers_sharpen.png`；公式：展示1组/行内2处 | 已覆盖；相关模式通过 |
| ### 模糊 | [模糊](05_framebuffers.md#模糊)（H3） | `05_framebuffers`：4 | `framebuffers_blur.png`；公式：展示1组/行内0处 | 已覆盖；相关模式通过 |
| ### 边缘检测 | [边缘检测](05_framebuffers.md#边缘检测)（H3） | `05_framebuffers`：5 | `framebuffers_edge_detection.png`；公式：展示1组/行内0处 | 已覆盖；相关模式通过 |
| ## 练习 | [练习](05_framebuffers.md#练习)（H2） | `05_framebuffers`：原后视镜/自选核练习 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已保留原练习 |

图片/公式：`container.jpg`、`framebuffers_screen_texture.png`、`framebuffers_inverse.png`、`framebuffers_grayscale.png`、`framebuffers_sharpen.png`、`framebuffers_blur.png`、`framebuffers_edge_detection.png`

非标题内容项：FBO/RBO/纹理完整链路、两种灰度、三个卷积案例、边界译注、两项练习；修正 RBO 不可读及核权重和表述。 实际摘录与截图见正文；练习保留为原练习，正文主案例未降为练习。


### 06 立方体贴图

底本文件：`docs/04 Advanced OpenGL/06 Cubemaps.md`。六面加载/尺寸顺序、天空盒基础与后画优化、模型反射折射、动态捕获概念、原反射贴图练习三项提示。

| 上游标题（原级别） | 本地对应 | 源码/模式与内容项 | 图示/公式 | 状态 |
| --- | --- | --- | --- | --- |
| # 立方体贴图 | [立方体贴图](06_cubemaps.md#立方体贴图)（H1） | `06_cubemaps`：0–3 | `cubemaps_sampling.png`、`cubemaps_morrowind.jpg`、`cubemaps_skybox.png`、`cubemaps_skybox_result.png`、`cubemaps_reflection_theory.png`、`cubemaps_reflection.png`、`cubemaps_reflection_backpack.png`、`cubemaps_refraction_theory.png`、`cubemaps_refraction_backpack.png`、`06_cubemaps.png`；公式：展示0组/行内7处 | 已覆盖；相关模式通过 |
| ## 创建立方体贴图 | [创建立方体贴图](06_cubemaps.md#创建立方体贴图)（H2） | `06_cubemaps`：load_cubemap；六面约定 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖；相关模式通过 |
| # 天空盒 | [天空盒](06_cubemaps.md#天空盒)（H2） | `06_cubemaps`：0–3 的背景 | `cubemaps_morrowind.jpg`、`cubemaps_skybox.png`、`cubemaps_skybox_result.png`；公式：展示0组/行内1处 | 已覆盖；相关模式通过 |
| ## 加载天空盒 | [加载天空盒](06_cubemaps.md#加载天空盒)（H3） | `06_cubemaps`：六张 JPG 解码/上传 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖；相关模式通过 |
| ## 显示天空盒 | [显示天空盒](06_cubemaps.md#显示天空盒)（H3） | `06_cubemaps`：sky program；去除平移 | `cubemaps_skybox_result.png` | 已覆盖；相关模式通过 |
| ## 优化 | [优化](06_cubemaps.md#优化)（H3） | `06_cubemaps`：所有模式；后画与 LEQUAL | ；公式：展示0组/行内1处 | 已覆盖；相关模式通过 |
| # 环境映射 | [环境映射](06_cubemaps.md#环境映射)（H2） | `06_cubemaps`：0–3 | `cubemaps_reflection_theory.png`、`cubemaps_reflection.png`、`cubemaps_reflection_backpack.png`、`cubemaps_refraction_theory.png`、`cubemaps_refraction_backpack.png`；公式：展示0组/行内6处 | 已覆盖；相关模式通过 |
| ## 反射 | [反射](06_cubemaps.md#反射)（H3） | `06_cubemaps`：0 / 2 | `cubemaps_reflection_theory.png`、`cubemaps_reflection.png`、`cubemaps_reflection_backpack.png`；公式：展示0组/行内4处 | 已覆盖；相关模式通过 |
| ## 折射 | [折射](06_cubemaps.md#折射)（H3） | `06_cubemaps`：1 / 3 | `cubemaps_refraction_theory.png`、`cubemaps_refraction_backpack.png`；公式：展示0组/行内2处 | 已覆盖；相关模式通过 |
| ## 动态环境贴图 | [动态环境贴图](06_cubemaps.md#动态环境贴图)（H3） | `06_cubemaps`：六遍捕获流程，原文概念 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖（概念） |
| ## 练习 | [练习](06_cubemaps.md#练习)（H2） | `06_cubemaps`：原反射图练习；三项提示 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已保留原练习 |

图片/公式：`cubemaps_sampling.png`、`cubemaps_morrowind.jpg`、`cubemaps_skybox.png`、`cubemaps_skybox_result.png`、`cubemaps_reflection_theory.png`、`cubemaps_reflection.png`、`cubemaps_reflection_backpack.png`、`cubemaps_refraction_theory.png`、`cubemaps_refraction_backpack.png`、`cubemaps_reflection_map.png`

非标题内容项：六面加载/尺寸顺序、天空盒基础与后画优化、模型反射折射、动态捕获概念、原反射贴图练习三项提示。 实际摘录与截图见正文；练习保留为原练习，正文主案例未降为练习。


### 07 高级数据

底本文件：`docs/04 Advanced OpenGL/07 Advanced Data.md`。分配/子区、完整与范围映射、分块位置/颜色与推广到法线/UV、双绑定复制；偏移范围用半开区间。

| 上游标题（原级别） | 本地对应 | 源码/模式与内容项 | 图示/公式 | 状态 |
| --- | --- | --- | --- | --- |
| # 高级数据 | [高级数据](07_advanced_data.md#高级数据)（H1） | `07_advanced_data`：0 / 1；分配/子区/映射 | `07_advanced_data.png`；公式：展示0组/行内3处 | 已覆盖；相关模式通过 |
| ## 分批顶点属性 | [分批顶点属性](07_advanced_data.md#分批顶点属性)（H2） | `07_advanced_data`：位置和颜色分块；法线/UV 推广 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖；相关模式通过 |
| ## 复制缓冲 | [复制缓冲](07_advanced_data.md#复制缓冲)（H2） | `07_advanced_data`：两绑定目标复制；原替代绑定说明 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖；相关模式通过 |

图片/公式：上游无图片；本仓库新增数据流全景图。

非标题内容项：分配/子区、完整与范围映射、分块位置/颜色与推广到法线/UV、双绑定复制；偏移范围用半开区间。 实际摘录与截图见正文；练习保留为原练习，正文主案例未降为练习。


### 08 高级 GLSL

底本文件：`docs/04 Advanced OpenGL/08 Advanced GLSL.md`。全部内建变量、保守深度版本边界、接口块、std140 规则和原偏移例、绑定点/Base/Range、四程序共享；总块尾对齐修正为160字节。

| 上游标题（原级别） | 本地对应 | 源码/模式与内容项 | 图示/公式 | 状态 |
| --- | --- | --- | --- | --- |
| # 高级GLSL | [高级 GLSL](08_advanced_glsl.md#高级-glsl)（H1） | `08_advanced_glsl`：0–4 | `advanced_glsl_pointsize.png`、`advanced_glsl_fragcoord.png`、`advanced_glsl_frontfacing.png`、`advanced_glsl_binding_points.png`、`advanced_glsl_uniform_buffer_objects.png`、`08_advanced_glsl.png`；公式：展示0组/行内1处 | 已覆盖；相关模式通过 |
| # GLSL的内建变量 | [GLSL 的内建变量](08_advanced_glsl.md#glsl-的内建变量)（H2） | `08_advanced_glsl`：1–4 | `advanced_glsl_pointsize.png`、`advanced_glsl_fragcoord.png`、`advanced_glsl_frontfacing.png` | 已覆盖；相关模式通过 |
| ## 顶点着色器变量 | [顶点着色器变量](08_advanced_glsl.md#顶点着色器变量)（H3） | `08_advanced_glsl`：1；顶点位置/大小/ID | `advanced_glsl_pointsize.png` | 已覆盖；相关模式通过 |
| ### gl_PointSize | [gl_PointSize](08_advanced_glsl.md#gl_pointsize)（H4） | `08_advanced_glsl`：1 | `advanced_glsl_pointsize.png` | 已覆盖；相关模式通过 |
| ### gl_VertexID | [gl_VertexID](08_advanced_glsl.md#gl_vertexid)（H4） | `08_advanced_glsl`：1 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖；相关模式通过 |
| ## 片段着色器变量 | [片段着色器变量](08_advanced_glsl.md#片段着色器变量)（H3） | `08_advanced_glsl`：2–4 | `advanced_glsl_fragcoord.png`、`advanced_glsl_frontfacing.png` | 已覆盖；相关模式通过 |
| ### gl_FragCoord | [gl_FragCoord](08_advanced_glsl.md#gl_fragcoord)（H4） | `08_advanced_glsl`：2 | `advanced_glsl_fragcoord.png` | 已覆盖；相关模式通过 |
| ### gl_FrontFacing | [gl_FrontFacing](08_advanced_glsl.md#gl_frontfacing)（H4） | `08_advanced_glsl`：3；可移动相机观察背面 | `advanced_glsl_frontfacing.png` | 已覆盖；相关模式通过 |
| ### gl_FragDepth | [gl_FragDepth](08_advanced_glsl.md#gl_fragdepth)（H4） | `08_advanced_glsl`：4；4.2保守深度另作概念 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖（概念） |
| # 接口块 | [接口块](08_advanced_glsl.md#接口块)（H2） | `08_advanced_glsl`：全部模式 VertexData | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖；相关模式通过 |
| # Uniform缓冲对象 | [Uniform 缓冲对象](08_advanced_glsl.md#uniform-缓冲对象)（H2） | `08_advanced_glsl`：0；Matrices | `advanced_glsl_binding_points.png`、`advanced_glsl_uniform_buffer_objects.png`；公式：展示0组/行内1处 | 已覆盖；相关模式通过 |
| ## Uniform块布局 | [Uniform 块布局](08_advanced_glsl.md#uniform-块布局)（H3） | `08_advanced_glsl`：std140 表/原偏移推导练习 | ；公式：展示0组/行内1处 | 已覆盖；相关模式通过 |
| ## 使用Uniform缓冲 | [使用 Uniform 缓冲](08_advanced_glsl.md#使用-uniform-缓冲)（H3） | `08_advanced_glsl`：四 program 块索引→绑定点→UBO；Range 说明 | `advanced_glsl_binding_points.png` | 已覆盖；相关模式通过 |
| ## 一个简单的例子 | [一个简单的例子](08_advanced_glsl.md#一个简单的例子)（H3） | `08_advanced_glsl`：0；四立方体共享投影/观察 | `advanced_glsl_uniform_buffer_objects.png` | 已覆盖；相关模式通过 |

图片/公式：`advanced_glsl_pointsize.png`、`advanced_glsl_fragcoord.png`、`advanced_glsl_frontfacing.png`、`advanced_glsl_binding_points.png`、`advanced_glsl_uniform_buffer_objects.png`

非标题内容项：全部内建变量、保守深度版本边界、接口块、std140 规则和原偏移例、绑定点/Base/Range、四程序共享；总块尾对齐修正为160字节。 实际摘录与截图见正文；练习保留为原练习，正文主案例未降为练习。


### 09 几何着色器

底本文件：`docs/04 Advanced OpenGL/09 Geometry Shader.md`。点/线、绿色→彩色→雪顶房子、真实模型爆破、普通模型+法线叠加；逐次重写输出、投影前求法线，无性能保证。

| 上游标题（原级别） | 本地对应 | 源码/模式与内容项 | 图示/公式 | 状态 |
| --- | --- | --- | --- | --- |
| # 几何着色器 | [几何着色器](09_geometry_shader.md#几何着色器)（H1） | `09_geometry_shader`：2；输入/输出拓扑表 | `geometry_shader_line_strip.png`、`geometry_shader_lines.png`、`geometry_shader_points.png`、`geometry_shader_points.png`、`geometry_shader_triangle_strip.png`、`geometry_shader_house.png`、`geometry_shader_houses.png`、`geometry_shader_houses_colored.png`、`geometry_shader_houses_snow.png`、`geometry_shader_explosion_backpack.png`、`geometry_shader_normals_backpack.png`、`09_geometry_shader.png`；公式：展示0组/行内11处 | 已覆盖；相关模式通过 |
| ## 使用几何着色器 | [使用几何着色器](09_geometry_shader.md#使用几何着色器)（H2） | `09_geometry_shader`：1；点传递与编译链接 | `geometry_shader_points.png`、`geometry_shader_points.png` | 已覆盖；相关模式通过 |
| ## 造几个房子 | [造几个房子](09_geometry_shader.md#造几个房子)（H2） | `09_geometry_shader`：6→3→0；基础/颜色/雪顶 | `geometry_shader_triangle_strip.png`、`geometry_shader_house.png`、`geometry_shader_houses.png`、`geometry_shader_houses_colored.png`、`geometry_shader_houses_snow.png` | 已覆盖；相关模式通过 |
| # 爆破物体 | [爆破物体](09_geometry_shader.md#爆破物体)（H2） | `09_geometry_shader`：4；实际导入模型与 UV | `geometry_shader_explosion_backpack.png`；公式：展示0组/行内8处 | 已覆盖；相关模式通过 |
| # 法向量可视化 | [法向量可视化](09_geometry_shader.md#法向量可视化)（H2） | `09_geometry_shader`：5；无GS基础绘制+法线叠加 | `geometry_shader_normals_backpack.png`；公式：展示0组/行内3处 | 已覆盖；相关模式通过 |

图片/公式：`geometry_shader_line_strip.png`、`geometry_shader_lines.png`、`geometry_shader_points.png`、`geometry_shader_triangle_strip.png`、`geometry_shader_house.png`、`geometry_shader_houses.png`、`geometry_shader_houses_colored.png`、`geometry_shader_houses_snow.png`、`geometry_shader_explosion_backpack.png`、`geometry_shader_normals_backpack.png`

非标题内容项：点/线、绿色→彩色→雪顶房子、真实模型爆破、普通模型+法线叠加；逐次重写输出、投影前求法线，无性能保证。 实际摘录与截图见正文；练习保留为原练习，正文主案例未降为练习。


### 10 实例化

底本文件：`docs/04 Advanced OpenGL/10 Instancing.md`。uniform 数组→实例数组→ID 尺寸渐变、普通与矩阵实例化小行星、四列位置3–6、每mesh配置与索引绘制；弧度和绘制次数修正。

| 上游标题（原级别） | 本地对应 | 源码/模式与内容项 | 图示/公式 | 状态 |
| --- | --- | --- | --- | --- |
| # 实例化 | [实例化](10_instancing.md#实例化)（H1） | `10_instancing`：1；uniform 数组与 ID | `instancing_quads.png`、`instancing_quads.png`、`instancing_quads_arrays.png`、`instancing_asteroids.png`、`instancing_asteroids_quantity.png`、`10_instancing.png`；公式：展示1组/行内6处 | 已覆盖；相关模式通过 |
| ## 实例化数组 | [实例化数组](10_instancing.md#实例化数组)（H2） | `10_instancing`：0 / 4；实例数组与尺寸渐变 | `instancing_quads.png`、`instancing_quads_arrays.png` | 已覆盖；相关模式通过 |
| # 小行星带 | [小行星带](10_instancing.md#小行星带)（H2） | `10_instancing`：2 / 3；普通/矩阵实例化索引模型 | `instancing_asteroids.png`、`instancing_asteroids_quantity.png`；公式：展示1组/行内6处 | 已覆盖；相关模式通过 |

图片/公式：`instancing_quads.png`、`instancing_quads_arrays.png`、`instancing_asteroids.png`、`instancing_asteroids_quantity.png`

非标题内容项：uniform 数组→实例数组→ID 尺寸渐变、普通与矩阵实例化小行星、四列位置3–6、每mesh配置与索引绘制；弧度和绘制次数修正。 实际摘录与截图见正文；练习保留为原练习，正文主案例未降为练习。


### 11 抗锯齿

底本文件：`docs/04 Advanced OpenGL/11 Anti Aliasing.md`。SSAA/MSAA 原理、覆盖图组、默认窗口/两种离屏附件/blit后处理/逐样本读取；修正采样数、着色次数和 sampler2DMS 坐标语义。

| 上游标题（原级别） | 本地对应 | 源码/模式与内容项 | 图示/公式 | 状态 |
| --- | --- | --- | --- | --- |
| # 抗锯齿 | [抗锯齿](11_anti_aliasing.md#抗锯齿)（H1） | `11_anti_aliasing`：走样/SSAA/MSAA 原理 | `anti_aliasing_aliasing.png`、`anti_aliasing_zoomed.png`、`anti_aliasing_rasterization.png`、`anti_aliasing_rasterization_filled.png`、`anti_aliasing_sample_points.png`、`anti_aliasing_rasterization_samples.png`、`anti_aliasing_rasterization_samples_filled.png`、`anti_aliasing_multisampled.png`、`anti_aliasing_multisampled.png`、`anti_aliasing_post_processing_grayscale.png`、`11_anti_aliasing.png` | 已覆盖；相关模式通过 |
| ## 多重采样 | [多重采样](11_anti_aliasing.md#多重采样)（H2） | `11_anti_aliasing`：覆盖率、深度/模板样本图组 | `anti_aliasing_rasterization.png`、`anti_aliasing_rasterization_filled.png`、`anti_aliasing_sample_points.png`、`anti_aliasing_rasterization_samples.png`、`anti_aliasing_rasterization_samples_filled.png` | 已覆盖；相关模式通过 |
| ## OpenGL中的MSAA | [OpenGL 中的 MSAA](11_anti_aliasing.md#opengl-中的-msaa)（H2） | `11_anti_aliasing`：3；默认 GLFW 样本查询 | `anti_aliasing_multisampled.png` | 已覆盖；相关模式通过 |
| ## 离屏MSAA | [离屏 MSAA](11_anti_aliasing.md#离屏-msaa)（H2） | `11_anti_aliasing`：0 / 2 / 4 | `anti_aliasing_multisampled.png`、`anti_aliasing_post_processing_grayscale.png` | 已覆盖；相关模式通过 |
| ### 多重采样纹理附件 | [多重采样纹理附件](11_anti_aliasing.md#多重采样纹理附件)（H3） | `11_anti_aliasing`：2；多采样颜色纹理 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖；相关模式通过 |
| ### 多重采样渲染缓冲对象 | [多重采样渲染缓冲对象](11_anti_aliasing.md#多重采样渲染缓冲对象)（H3） | `11_anti_aliasing`：0 / 4；多采样 RBO | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖；相关模式通过 |
| ### 渲染到多重采样帧缓冲 | [渲染到多重采样帧缓冲](11_anti_aliasing.md#渲染到多重采样帧缓冲)（H3） | `11_anti_aliasing`：0 / 4；blit 与后处理 | `anti_aliasing_multisampled.png`、`anti_aliasing_post_processing_grayscale.png` | 已覆盖；相关模式通过 |
| ## 自定义抗锯齿算法 | [自定义抗锯齿算法](11_anti_aliasing.md#自定义抗锯齿算法)（H2） | `11_anti_aliasing`：2；texelFetch 样本平均 | 见正文 API/状态表、代码或概念步骤；本节无独立图 | 已覆盖；相关模式通过 |

图片/公式：`anti_aliasing_aliasing.png`、`anti_aliasing_zoomed.png`、`anti_aliasing_rasterization.png`、`anti_aliasing_rasterization_filled.png`、`anti_aliasing_sample_points.png`、`anti_aliasing_rasterization_samples.png`、`anti_aliasing_rasterization_samples_filled.png`、`anti_aliasing_multisampled.png`、`anti_aliasing_post_processing_grayscale.png`

非标题内容项：SSAA/MSAA 原理、覆盖图组、默认窗口/两种离屏附件/blit后处理/逐样本读取；修正采样数、着色次数和 sampler2DMS 坐标语义。 实际摘录与截图见正文；练习保留为原练习，正文主案例未降为练习。


## 资源与新增补充

- 上游标题映射共 74 项，11 篇全部对应；图片及行内链接教学资源共 67 个，均已本地复制，文件与 blob 来源见[图片记录](../img/04/README.md)。
- 原有作者/译者/校对署名保留在每篇元数据；源码与附加资源链接集中列出并在相关正文保留用途。
- 本仓库补充：每篇核心句、职责边界、全景图、常见误解、1 个进阶块、模式表、实测图和整体回顾。补充项没有替代上游条目。
- 本次只核验第 04 章；前 01–03 章是否满足后来加强的逐项覆盖契约，需单独核验。
