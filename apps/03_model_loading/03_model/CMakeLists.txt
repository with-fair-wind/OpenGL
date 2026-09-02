add_executable(03_model_loading__03_model main.cpp)

opengl_lab_apply_common_options(03_model_loading__03_model)

target_compile_definitions(
    03_model_loading__03_model
    PRIVATE
        OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)

target_link_libraries(
    03_model_loading__03_model
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
        stb::stb
        assimp::assimp
)
