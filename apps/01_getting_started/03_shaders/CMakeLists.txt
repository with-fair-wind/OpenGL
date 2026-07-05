add_executable(01_getting_started__03_shaders main.cpp)

opengl_lab_apply_common_options(01_getting_started__03_shaders)

target_link_libraries(
    01_getting_started__03_shaders
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
