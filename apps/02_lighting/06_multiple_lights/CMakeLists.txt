add_executable(02_lighting__06_multiple_lights main.cpp)

opengl_lab_apply_common_options(02_lighting__06_multiple_lights)

target_compile_definitions(
    02_lighting__06_multiple_lights
    PRIVATE
        OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)

target_link_libraries(
    02_lighting__06_multiple_lights
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
        stb::stb
)
