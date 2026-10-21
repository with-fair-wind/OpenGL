add_executable(04_advanced_opengl__01_depth_testing main.cpp)

opengl_lab_apply_common_options(04_advanced_opengl__01_depth_testing)

target_link_libraries(
    04_advanced_opengl__01_depth_testing
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
