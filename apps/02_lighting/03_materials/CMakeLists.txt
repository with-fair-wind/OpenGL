add_executable(02_lighting__03_materials main.cpp)

opengl_lab_apply_common_options(02_lighting__03_materials)

target_link_libraries(
    02_lighting__03_materials
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)