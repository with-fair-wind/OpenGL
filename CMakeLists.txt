cmake_minimum_required(VERSION 3.28)

project(
    OpenGLLab
    VERSION 0.1.0
    DESCRIPTION "A modern C++ OpenGL learning workspace"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(OpenGLLabOptions)

opengl_lab_setup_options()
opengl_lab_setup_clangd_support()

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(stb CONFIG REQUIRED)

if(OPENGL_LAB_BUILD_EXAMPLES)
    add_subdirectory(apps)
endif()
