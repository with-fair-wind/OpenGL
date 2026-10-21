add_executable(04_advanced_opengl__04_face_culling main.cpp)

opengl_lab_apply_common_options(04_advanced_opengl__04_face_culling)

target_link_libraries(
    04_advanced_opengl__04_face_culling
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
