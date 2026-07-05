add_executable(01_getting_started__02_hello_triangle main.cpp)

opengl_lab_apply_common_options(01_getting_started__02_hello_triangle)

target_link_libraries(
    01_getting_started__02_hello_triangle
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
