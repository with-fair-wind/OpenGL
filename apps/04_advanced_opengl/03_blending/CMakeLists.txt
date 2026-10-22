add_executable(04_advanced_opengl__03_blending main.cpp)

opengl_lab_apply_common_options(04_advanced_opengl__03_blending)

target_link_libraries(
    04_advanced_opengl__03_blending
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
        stb::stb
)

target_compile_definitions(04_advanced_opengl__03_blending PRIVATE OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets")
