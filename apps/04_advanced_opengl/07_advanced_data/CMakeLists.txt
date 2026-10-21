add_executable(04_advanced_opengl__07_advanced_data main.cpp)

opengl_lab_apply_common_options(04_advanced_opengl__07_advanced_data)

target_link_libraries(
    04_advanced_opengl__07_advanced_data
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
