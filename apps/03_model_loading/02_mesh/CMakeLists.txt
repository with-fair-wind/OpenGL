add_executable(03_model_loading__02_mesh main.cpp)

opengl_lab_apply_common_options(03_model_loading__02_mesh)

target_compile_definitions(
    03_model_loading__02_mesh
    PRIVATE
        OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)

target_link_libraries(
    03_model_loading__02_mesh
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
        stb::stb
)
