add_executable(04_advanced_opengl__09_geometry_shader main.cpp)

opengl_lab_apply_common_options(04_advanced_opengl__09_geometry_shader)

target_link_libraries(
    04_advanced_opengl__09_geometry_shader
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
