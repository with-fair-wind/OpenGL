add_executable(01_getting_started__01_hello_window main.cpp)

opengl_lab_apply_common_options(01_getting_started__01_hello_window)

target_link_libraries(
    01_getting_started__01_hello_window
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
