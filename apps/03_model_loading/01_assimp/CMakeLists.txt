add_executable(03_model_loading__01_assimp main.cpp)

opengl_lab_apply_common_options(03_model_loading__01_assimp)

target_compile_definitions(
    03_model_loading__01_assimp
    PRIVATE
        OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)

target_link_libraries(
    03_model_loading__01_assimp
    PRIVATE
        glad::glad
        glfw
        glm::glm-header-only
        assimp::assimp
)
