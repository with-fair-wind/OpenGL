# 第 05 章程序资源

本章直接复用已有 stb_image 与 Assimp 依赖，不增加 Conan 包。以下纹理来自 [JoeyDeVries/LearnOpenGL](https://github.com/JoeyDeVries/LearnOpenGL) 固定提交 `a545a703f95893258d16dbe32f5ccbb6400fd213`，原始资源的出处与许可说明遵循上游仓库；颜色图按 sRGB，法线、深度图按线性参数读取。

| 本地文件 | 固定版本来源 | Git blob SHA |
| --- | --- | --- |
| [textures/advanced_lighting/wood.png](textures/advanced_lighting/wood.png) | [原文件](https://github.com/JoeyDeVries/LearnOpenGL/blob/a545a703f95893258d16dbe32f5ccbb6400fd213/resources/textures/wood.png) | `c785f41f2f54b3d3cae5bee297ef5af7b0598374` |
| [textures/advanced_lighting/brickwall.jpg](textures/advanced_lighting/brickwall.jpg) | [原文件](https://github.com/JoeyDeVries/LearnOpenGL/blob/a545a703f95893258d16dbe32f5ccbb6400fd213/resources/textures/brickwall.jpg) | `39478d3eeb094043d8e7f954105ffb070efc50d0` |
| [textures/advanced_lighting/brickwall_normal.jpg](textures/advanced_lighting/brickwall_normal.jpg) | [原文件](https://github.com/JoeyDeVries/LearnOpenGL/blob/a545a703f95893258d16dbe32f5ccbb6400fd213/resources/textures/brickwall_normal.jpg) | `a0a5fb625d5aa3966de5dc05d7d7a190368ca4af` |
| [textures/advanced_lighting/bricks2.jpg](textures/advanced_lighting/bricks2.jpg) | [原文件](https://github.com/JoeyDeVries/LearnOpenGL/blob/a545a703f95893258d16dbe32f5ccbb6400fd213/resources/textures/bricks2.jpg) | `358350883f41dc8444b30a7d9686fa84f1113506` |
| [textures/advanced_lighting/bricks2_normal.jpg](textures/advanced_lighting/bricks2_normal.jpg) | [原文件](https://github.com/JoeyDeVries/LearnOpenGL/blob/a545a703f95893258d16dbe32f5ccbb6400fd213/resources/textures/bricks2_normal.jpg) | `aa6643dee352b01a469542450436203f73025aab` |
| [textures/advanced_lighting/bricks2_disp.jpg](textures/advanced_lighting/bricks2_disp.jpg) | [原文件](https://github.com/JoeyDeVries/LearnOpenGL/blob/a545a703f95893258d16dbe32f5ccbb6400fd213/resources/textures/bricks2_disp.jpg) | `48ab26fad0a324f9b3058336b8764aa507ff09f9` |
| [textures/advanced_lighting/toy_box_diffuse.png](textures/advanced_lighting/toy_box_diffuse.png) | [原文件](https://github.com/JoeyDeVries/LearnOpenGL/blob/a545a703f95893258d16dbe32f5ccbb6400fd213/resources/textures/toy_box_diffuse.png) | `e28e2aee3e6fe20fb8e3b4030df2af0794a766eb` |
| [textures/advanced_lighting/toy_box_normal.png](textures/advanced_lighting/toy_box_normal.png) | [原文件](https://github.com/JoeyDeVries/LearnOpenGL/blob/a545a703f95893258d16dbe32f5ccbb6400fd213/resources/textures/toy_box_normal.png) | `8b39dff48870bbff5204c8f6cf292944a605d196` |
| [textures/advanced_lighting/toy_box_disp.png](textures/advanced_lighting/toy_box_disp.png) | [原文件](https://github.com/JoeyDeVries/LearnOpenGL/blob/a545a703f95893258d16dbe32f5ccbb6400fd213/resources/textures/toy_box_disp.png) | `230bbbe37cfb26ddb3d924e89d15e3b65c2f5c64` |

`models/relief_box/relief_box.obj` 复用本仓库手写 crate 几何及 UV，MTL 将官方砖墙颜色/法线与已有教学镜面遮罩关联。map_Bump 在这个特定材质中明确表示切线空间法线，不能推广为任意 OBJ bump 都是法线图。09/10 直接导入已有 crate OBJ 并复用其颜色/镜面 PPM，未下载上游复杂人物模型。

01/02/03/04/07/08 的木纹来自同一个 wood.png；05 使用 brickwall 组，06 的砖墙与玩具箱各使用配套颜色、法线、深度三图。纹理不做额外的逐张通道翻转，切线方向与模型 UV 一起解释。
