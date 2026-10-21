add_executable(04_advanced_opengl__11_anti_aliasing main.cpp)

opengl_lab_apply_common_options(04_advanced_opengl__11_anti_aliasing)

target_link_libraries(
    04_advanced_opengl__11_anti_aliasing
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
