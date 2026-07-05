add_executable(01_getting_started__04_textures main.cpp)

opengl_lab_apply_common_options(01_getting_started__04_textures)

target_compile_definitions(
    01_getting_started__04_textures
    PRIVATE
        OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)

target_link_libraries(
    01_getting_started__04_textures
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
        stb::stb
)
