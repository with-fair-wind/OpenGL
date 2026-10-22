add_executable(05_advanced_lighting__05_normal_mapping main.cpp)

opengl_lab_apply_common_options(05_advanced_lighting__05_normal_mapping)

target_link_libraries(05_advanced_lighting__05_normal_mapping PRIVATE glad::glad glfw glm::glm-header-only stb::stb assimp::assimp)

target_compile_definitions(05_advanced_lighting__05_normal_mapping PRIVATE OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets")
