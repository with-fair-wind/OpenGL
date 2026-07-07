add_executable(02_lighting__02_basic_lighting main.cpp)

opengl_lab_apply_common_options(02_lighting__02_basic_lighting)

target_link_libraries(
    02_lighting__02_basic_lighting
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)