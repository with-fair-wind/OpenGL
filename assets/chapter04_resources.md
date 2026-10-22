# 第 04 章程序资源

这些资源与教程原理图分别记录。资源根由 `OPENGL_LAB_ASSET_ROOT` 指向，不依赖启动工作目录。

| 本地文件 | 来源 / 用途 | Git blob SHA |
| --- | --- | --- |
| [textures/skybox/back.jpg](textures/skybox/back.jpg) | [LearnOpenGL 原文件](https://api.github.com/repos/JoeyDeVries/LearnOpenGL/git/blobs/470a67973f10e14c590332d744761527f964442a)；blob 见右列 | `470a67973f10e14c590332d744761527f964442a` |
| [textures/skybox/bottom.jpg](textures/skybox/bottom.jpg) | [LearnOpenGL 原文件](https://api.github.com/repos/JoeyDeVries/LearnOpenGL/git/blobs/893f394788a3b4e4f27498e4bcde38da93416ab0)；blob 见右列 | `893f394788a3b4e4f27498e4bcde38da93416ab0` |
| [textures/skybox/front.jpg](textures/skybox/front.jpg) | [LearnOpenGL 原文件](https://api.github.com/repos/JoeyDeVries/LearnOpenGL/git/blobs/4e17b77999ac0cea47d75a9dfee2022c1b5d112e)；blob 见右列 | `4e17b77999ac0cea47d75a9dfee2022c1b5d112e` |
| [textures/skybox/left.jpg](textures/skybox/left.jpg) | [LearnOpenGL 原文件](https://api.github.com/repos/JoeyDeVries/LearnOpenGL/git/blobs/5750b91a1c2c55358b276822990a52b766057bad)；blob 见右列 | `5750b91a1c2c55358b276822990a52b766057bad` |
| [textures/skybox/right.jpg](textures/skybox/right.jpg) | [LearnOpenGL 原文件](https://api.github.com/repos/JoeyDeVries/LearnOpenGL/git/blobs/89630371d981ab8aa5458844798061ad917d85c7)；blob 见右列 | `89630371d981ab8aa5458844798061ad917d85c7` |
| [textures/skybox/top.jpg](textures/skybox/top.jpg) | [LearnOpenGL 原文件](https://api.github.com/repos/JoeyDeVries/LearnOpenGL/git/blobs/4db3c2a3fa76758a3d3dda933e88e35084f68767)；blob 见右列 | `4db3c2a3fa76758a3d3dda933e88e35084f68767` |

- [grass.png](textures/grass.png)：LearnOpenGL-CN 固定提交 `331560886468e28f28326429334fc48470f9c27b` 的 `docs/img/04/03/grass.png`，原样复制，供 alpha 丢弃演示。
- [window.png](textures/window.png)：同版本 `docs/img/04/03/blending_transparent_window.png`，原样复制，供半透明混合演示。
- [planet.obj](models/asteroid/planet.obj)、[rock.obj](models/asteroid/rock.obj)：本仓库生成的球面与受扰动球面教学模型，分别 1024 / 120 个三角形，无第三方模型来源；Assimp 读取后生成法线并建立索引网格。
- `models/crate/`、`models/stage/` 及 PPM 纹理是仓库原有资源，几何着色器和环境映射案例沿用。

六面上传顺序为 right/left/top/bottom/front/back，对应 +X/-X/+Y/-Y/+Z/-Z。图片原作者、教程作者及上游许可见[LearnOpenGL](https://github.com/JoeyDeVries/LearnOpenGL)与[中文底本](https://github.com/LearnOpenGL-CN/LearnOpenGL-CN)；本地副本保留教学来源，不宣称原创摄影。
