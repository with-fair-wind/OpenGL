add_executable(01_getting_started__07_camera main.cpp)

opengl_lab_apply_common_options(01_getting_started__07_camera)

target_compile_definitions(
    01_getting_started__07_camera
    PRIVATE
        OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)

target_link_libraries(
    01_getting_started__07_camera
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
        stb::stb
)