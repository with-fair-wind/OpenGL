add_executable(01_getting_started__06_coordinate_systems main.cpp)

opengl_lab_apply_common_options(01_getting_started__06_coordinate_systems)

target_compile_definitions(
    01_getting_started__06_coordinate_systems
    PRIVATE
        OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)

target_link_libraries(
    01_getting_started__06_coordinate_systems
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
        stb::stb
)