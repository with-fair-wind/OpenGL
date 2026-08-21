add_executable(02_lighting__04_lighting_maps main.cpp)

opengl_lab_apply_common_options(02_lighting__04_lighting_maps)

target_compile_definitions(
    02_lighting__04_lighting_maps
    PRIVATE
        OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)

target_link_libraries(
    02_lighting__04_lighting_maps
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
        stb::stb
)
