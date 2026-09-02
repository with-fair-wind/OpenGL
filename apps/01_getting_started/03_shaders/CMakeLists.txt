add_executable(01_getting_started__03_shaders main.cpp)
add_executable(01_getting_started__03_shaders_exercise1 exercise1.cpp)
add_executable(01_getting_started__03_shaders_exercise2 exercise2.cpp)
add_executable(01_getting_started__03_shaders_exercise3 exercise3.cpp)

opengl_lab_apply_common_options(01_getting_started__03_shaders)
opengl_lab_apply_common_options(01_getting_started__03_shaders_exercise1)
opengl_lab_apply_common_options(01_getting_started__03_shaders_exercise2)
opengl_lab_apply_common_options(01_getting_started__03_shaders_exercise3)

target_link_libraries(
    01_getting_started__03_shaders
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
target_link_libraries(
    01_getting_started__03_shaders_exercise1
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
target_link_libraries(
    01_getting_started__03_shaders_exercise2
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
target_link_libraries(
    01_getting_started__03_shaders_exercise3
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
)
