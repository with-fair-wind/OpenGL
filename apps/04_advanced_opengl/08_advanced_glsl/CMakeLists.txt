add_executable(04_advanced_opengl__08_advanced_glsl main.cpp)

opengl_lab_apply_common_options(04_advanced_opengl__08_advanced_glsl)

target_link_libraries(
    04_advanced_opengl__08_advanced_glsl
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
