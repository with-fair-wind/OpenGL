add_executable(06_pbr__04_specular_ibl main.cpp)
opengl_lab_apply_common_options(06_pbr__04_specular_ibl)
target_link_libraries(06_pbr__04_specular_ibl PRIVATE glad::glad glfw glm::glm-header-only stb::stb assimp::assimp)
target_compile_definitions(06_pbr__04_specular_ibl PRIVATE
    OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)
