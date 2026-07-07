add_executable(02_lighting__01_colors main.cpp)

opengl_lab_apply_common_options(02_lighting__01_colors)

target_link_libraries(
    02_lighting__01_colors
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)