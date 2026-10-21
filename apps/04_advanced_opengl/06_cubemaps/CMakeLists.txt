add_executable(04_advanced_opengl__06_cubemaps main.cpp)

opengl_lab_apply_common_options(04_advanced_opengl__06_cubemaps)

target_link_libraries(
    04_advanced_opengl__06_cubemaps
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
