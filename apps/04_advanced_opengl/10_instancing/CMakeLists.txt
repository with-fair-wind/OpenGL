add_executable(04_advanced_opengl__10_instancing main.cpp)

opengl_lab_apply_common_options(04_advanced_opengl__10_instancing)

target_link_libraries(
    04_advanced_opengl__10_instancing
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
