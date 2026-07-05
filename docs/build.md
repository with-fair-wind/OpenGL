# 构建指南

本文记录 OpenGL Lab 的全部构建 preset、Conan 安装方式，以及单配置/多配置生成器的差异。

## 构建模型

本项目把 CMake preset 按生成器类型分成两类：

- 单配置生成器：`Ninja`。`Debug` / `Release` 在 configure 阶段由 `CMAKE_BUILD_TYPE` 决定，所以 configure preset 会区分 `*-debug` 和 `*-release`。
- 多配置生成器：`Ninja Multi-Config`、`Visual Studio 18 2026`。一次 configure 会生成多个配置，`Debug` / `Release` 在 build 阶段由 build preset 的 `configuration` 决定。本项目把多配置生成器的 `CMAKE_CONFIGURATION_TYPES` 限定为 `Debug;Release`，和 Conan 安装步骤保持一致。

对应关系：

| 类型 | Configure preset | Build preset |
| --- | --- | --- |
| 单配置 | `mingw-gcc-debug` | `mingw-gcc-debug` |
| 单配置 | `mingw-gcc-release` | `mingw-gcc-release` |
| 单配置 | `mingw-clang-debug` | `mingw-clang-debug` |
| 单配置 | `mingw-clang-release` | `mingw-clang-release` |
| 单配置 | `msvc-ninja-debug` | `msvc-ninja-debug` |
| 单配置 | `msvc-ninja-release` | `msvc-ninja-release` |
| 单配置 | `clang-cl-debug` | `clang-cl-debug` |
| 单配置 | `clang-cl-release` | `clang-cl-release` |
| 多配置 | `msvc-ninja-multi` | `msvc-ninja-multi-debug` / `msvc-ninja-multi-release` |
| 多配置 | `clang-cl-ninja-multi` | `clang-cl-ninja-multi-debug` / `clang-cl-ninja-multi-release` |
| 多配置 | `msvc-vs2026` | `msvc-vs2026-debug` / `msvc-vs2026-release` |

## 工具链要求

`CMakePresets.json` 和 `conan/profiles/*` 不写本机绝对路径。请先确保对应工具链在当前终端的 `PATH` 上。

- MinGW GCC: 需要 MSYS2 `ucrt64/bin`，并能找到 `x86_64-w64-mingw32-g++`。
- MSYS2 Clang: 需要 MSYS2 `clang64/bin`，并能找到 `x86_64-w64-mingw32-clang++`。
- MSVC + Ninja: 需要在 Visual Studio Developer PowerShell/Command Prompt 中运行，并能找到 `cl` 和 `ninja`。
- clang-cl + Ninja: 需要在 Visual Studio Developer PowerShell/Command Prompt 中运行，并能找到 `clang-cl` 和 `ninja`。
- MSVC + Visual Studio 18 2026: 需要安装 Visual Studio 18 2026，CMake generator 使用 `Visual Studio 18 2026`。

## clangd 与 compile_commands.json

项目开启了 `CMAKE_EXPORT_COMPILE_COMMANDS`。对 `Ninja` 和 `Ninja Multi-Config` 这类会生成编译数据库的生成器，CMake 会在构建时通过 `opengl_lab_mirror_compile_commands` 目标把 `build/<preset>/compile_commands.json` 镜像到源码根目录：

```text
compile_commands.json
```

根目录的 `.clangd` 已设置为从源码根目录读取编译数据库：

```yaml
CompileFlags:
  CompilationDatabase: .
```

因此通常只需要先完成对应 preset 的 configure/build，clangd 就能从源码根目录读到最新的编译参数。Visual Studio 生成器本身不导出 `compile_commands.json`，需要 clangd 时优先使用 `Ninja` 或 `Ninja Multi-Config` preset 生成编译数据库。

## Conan 与多配置生成器

单配置生成器每个 build type 使用独立输出目录，例如：

```text
build/mingw-gcc-debug
build/mingw-gcc-release
```

因为 `CMAKE_BUILD_TYPE` 在 configure 阶段已经固定，Debug 和 Release 本来就应该是两棵不同的构建树。

多配置生成器使用同一个输出目录，例如：

```text
build/msvc-vs2026
build/msvc-ninja-multi
build/clang-cl-ninja-multi
```

“分别为 Debug/Release 安装依赖到同一个 Conan 输出结构里”的意思是：对同一个 `-of build/<preset>` 连续运行两次 `conan install`，一次安装 Debug 依赖，一次安装 Release 依赖。Conan 会把两种配置的 CMakeDeps/CMakeToolchain 文件放进同一棵 `build/<preset>/build/generators` 中，CMake configure 时只需要指向这一处 `conan_toolchain.cmake`。

例如 `msvc-vs2026`：

```powershell
conan install . -of build/msvc-vs2026 -pr:h conan/profiles/msvc-vs2026 -pr:b conan/profiles/msvc-vs2026 -s:h build_type=Debug -s:b build_type=Debug -s:h compiler.runtime_type=Debug -s:b compiler.runtime_type=Debug --build=missing
conan install . -of build/msvc-vs2026 -pr:h conan/profiles/msvc-vs2026 -pr:b conan/profiles/msvc-vs2026 -s:h build_type=Release -s:b build_type=Release -s:h compiler.runtime_type=Release -s:b compiler.runtime_type=Release --build=missing
```

这样生成的文件会集中在类似下面的位置：

```text
build/msvc-vs2026/build/generators/conan_toolchain.cmake
build/msvc-vs2026/build/generators/*-config.cmake
build/msvc-vs2026/build/generators/*-Target-debug.cmake
build/msvc-vs2026/build/generators/*-Target-release.cmake
```

如果把 Debug 和 Release 分别安装到 `build/msvc-vs2026-debug` 和 `build/msvc-vs2026-release`，那么一个多配置 configure preset 只能指向其中一个 `conan_toolchain.cmake`，就失去了多配置生成器“一次 configure，多次 build”的意义。

## 单配置构建

单配置流程固定是：安装当前 build type 的 Conan 依赖，configure 当前 build type，build 当前 build type。

### MinGW GCC + Ninja

Debug:

```powershell
conan install . -of build/mingw-gcc-debug -pr:h conan/profiles/mingw-gcc -pr:b conan/profiles/mingw-gcc -s build_type=Debug --build=missing
cmake --preset mingw-gcc-debug
cmake --build --preset mingw-gcc-debug
```

Release:

```powershell
conan install . -of build/mingw-gcc-release -pr:h conan/profiles/mingw-gcc -pr:b conan/profiles/mingw-gcc -s build_type=Release --build=missing
cmake --preset mingw-gcc-release
cmake --build --preset mingw-gcc-release
```

### MSYS2 Clang + Ninja

Debug:

```powershell
conan install . -of build/mingw-clang-debug -pr:h conan/profiles/mingw-clang -pr:b conan/profiles/mingw-clang -s build_type=Debug --build=missing
cmake --preset mingw-clang-debug
cmake --build --preset mingw-clang-debug
```

Release:

```powershell
conan install . -of build/mingw-clang-release -pr:h conan/profiles/mingw-clang -pr:b conan/profiles/mingw-clang -s build_type=Release --build=missing
cmake --preset mingw-clang-release
cmake --build --preset mingw-clang-release
```

### MSVC + Ninja

Debug:

```powershell
conan install . -of build/msvc-ninja-debug -pr:h conan/profiles/msvc-ninja -pr:b conan/profiles/msvc-ninja -s:h build_type=Debug -s:b build_type=Debug -s:h compiler.runtime_type=Debug -s:b compiler.runtime_type=Debug --build=missing
cmake --preset msvc-ninja-debug
cmake --build --preset msvc-ninja-debug
```

Release:

```powershell
conan install . -of build/msvc-ninja-release -pr:h conan/profiles/msvc-ninja -pr:b conan/profiles/msvc-ninja -s:h build_type=Release -s:b build_type=Release -s:h compiler.runtime_type=Release -s:b compiler.runtime_type=Release --build=missing
cmake --preset msvc-ninja-release
cmake --build --preset msvc-ninja-release
```

### clang-cl + Ninja

Debug:

```powershell
conan install . -of build/clang-cl-debug -pr:h conan/profiles/clang-cl -pr:b conan/profiles/clang-cl -s:h build_type=Debug -s:b build_type=Debug -s:h compiler.runtime_type=Debug -s:b compiler.runtime_type=Debug --build=missing
cmake --preset clang-cl-debug
cmake --build --preset clang-cl-debug
```

Release:

```powershell
conan install . -of build/clang-cl-release -pr:h conan/profiles/clang-cl -pr:b conan/profiles/clang-cl -s:h build_type=Release -s:b build_type=Release -s:h compiler.runtime_type=Release -s:b compiler.runtime_type=Release --build=missing
cmake --preset clang-cl-release
cmake --build --preset clang-cl-release
```

## 多配置构建

多配置流程固定是：先把 Debug 和 Release 的 Conan 依赖安装到同一个输出目录，再 configure 一次，然后按需要 build Debug 或 Release。

### MSVC + Ninja Multi-Config

安装 Debug 和 Release 依赖：

```powershell
conan install . -of build/msvc-ninja-multi -pr:h conan/profiles/msvc-ninja-multi -pr:b conan/profiles/msvc-ninja-multi -s:h build_type=Debug -s:b build_type=Debug -s:h compiler.runtime_type=Debug -s:b compiler.runtime_type=Debug --build=missing
conan install . -of build/msvc-ninja-multi -pr:h conan/profiles/msvc-ninja-multi -pr:b conan/profiles/msvc-ninja-multi -s:h build_type=Release -s:b build_type=Release -s:h compiler.runtime_type=Release -s:b compiler.runtime_type=Release --build=missing
```

配置一次：

```powershell
cmake --preset msvc-ninja-multi
```

分别构建：

```powershell
cmake --build --preset msvc-ninja-multi-debug
cmake --build --preset msvc-ninja-multi-release
```

### clang-cl + Ninja Multi-Config

安装 Debug 和 Release 依赖：

```powershell
conan install . -of build/clang-cl-ninja-multi -pr:h conan/profiles/clang-cl-ninja-multi -pr:b conan/profiles/clang-cl-ninja-multi -s:h build_type=Debug -s:b build_type=Debug -s:h compiler.runtime_type=Debug -s:b compiler.runtime_type=Debug --build=missing
conan install . -of build/clang-cl-ninja-multi -pr:h conan/profiles/clang-cl-ninja-multi -pr:b conan/profiles/clang-cl-ninja-multi -s:h build_type=Release -s:b build_type=Release -s:h compiler.runtime_type=Release -s:b compiler.runtime_type=Release --build=missing
```

配置一次：

```powershell
cmake --preset clang-cl-ninja-multi
```

分别构建：

```powershell
cmake --build --preset clang-cl-ninja-multi-debug
cmake --build --preset clang-cl-ninja-multi-release
```

### MSVC + Visual Studio 18 2026

安装 Debug 和 Release 依赖：

```powershell
conan install . -of build/msvc-vs2026 -pr:h conan/profiles/msvc-vs2026 -pr:b conan/profiles/msvc-vs2026 -s:h build_type=Debug -s:b build_type=Debug -s:h compiler.runtime_type=Debug -s:b compiler.runtime_type=Debug --build=missing
conan install . -of build/msvc-vs2026 -pr:h conan/profiles/msvc-vs2026 -pr:b conan/profiles/msvc-vs2026 -s:h build_type=Release -s:b build_type=Release -s:h compiler.runtime_type=Release -s:b compiler.runtime_type=Release --build=missing
```

配置一次：

```powershell
cmake --preset msvc-vs2026
```

分别构建：

```powershell
cmake --build --preset msvc-vs2026-debug
cmake --build --preset msvc-vs2026-release
```

也可以直接打开生成的 Visual Studio solution：

```text
build/msvc-vs2026/OpenGLLab.sln
```

## 运行示例

构建完成后运行对应 preset 目录下的可执行文件。单配置生成器示例：

```powershell
.\build\mingw-gcc-debug\apps\01_getting_started\01_hello_window\01_getting_started__01_hello_window.exe
.\build\mingw-gcc-debug\apps\01_getting_started\02_hello_triangle\01_getting_started__02_hello_triangle.exe
.\build\mingw-gcc-debug\apps\01_getting_started\03_shaders\01_getting_started__03_shaders.exe
.\build\mingw-gcc-debug\apps\01_getting_started\04_textures\01_getting_started__04_textures.exe
```

多配置生成器通常会在目标目录下多一层配置名，例如 `Debug` 或 `Release`。以 Ninja Multi-Config 为例：

```powershell
.\build\msvc-ninja-multi\apps\01_getting_started\01_hello_window\Debug\01_getting_started__01_hello_window.exe
.\build\msvc-ninja-multi\apps\01_getting_started\02_hello_triangle\Debug\01_getting_started__02_hello_triangle.exe
.\build\msvc-ninja-multi\apps\01_getting_started\03_shaders\Debug\01_getting_started__03_shaders.exe
.\build\msvc-ninja-multi\apps\01_getting_started\04_textures\Debug\01_getting_started__04_textures.exe
```

窗口打开后，按 `Esc` 退出。纹理示例会从源码树的 `assets/textures/checker.ppm` 加载图片；如果移动仓库目录，请重新 configure/build，让 `OPENGL_LAB_ASSET_ROOT` 更新为新的路径。
