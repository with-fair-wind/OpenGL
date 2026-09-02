from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout


class OpenGLLabConan(ConanFile):
    name = "opengl_lab"
    version = "0.1.0"
    package_type = "application"

    settings = "os", "arch", "compiler", "build_type"

    def requirements(self):
        self.requires("glfw/3.4")
        self.requires("glad/0.1.36")
        self.requires("glm/1.0.1")
        # stb: assimp 传递依赖旧版 stb，这里强制整个依赖图使用项目版本（header-only，向前兼容）。
        self.requires("stb/cci.20240531", force=True)
        self.requires("assimp/5.4.3")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        toolchain = CMakeToolchain(self)
        toolchain.generate()

