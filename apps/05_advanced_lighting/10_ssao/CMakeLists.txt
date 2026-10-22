add_executable(05_advanced_lighting__10_ssao main.cpp)

opengl_lab_apply_common_options(05_advanced_lighting__10_ssao)

target_link_libraries(05_advanced_lighting__10_ssao PRIVATE glad::glad glfw glm::glm-header-only stb::stb assimp::assimp)

target_compile_definitions(05_advanced_lighting__10_ssao PRIVATE OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets")
