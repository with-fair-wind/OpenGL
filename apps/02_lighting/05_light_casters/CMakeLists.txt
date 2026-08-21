add_executable(02_lighting__05_light_casters main.cpp)

opengl_lab_apply_common_options(02_lighting__05_light_casters)

target_compile_definitions(
    02_lighting__05_light_casters
    PRIVATE
        OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)

target_link_libraries(
    02_lighting__05_light_casters
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
        stb::stb
)
