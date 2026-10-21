add_executable(04_advanced_opengl__05_framebuffers main.cpp)

opengl_lab_apply_common_options(04_advanced_opengl__05_framebuffers)

target_link_libraries(
    04_advanced_opengl__05_framebuffers
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
