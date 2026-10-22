add_executable(05_advanced_lighting__08_bloom main.cpp)

opengl_lab_apply_common_options(05_advanced_lighting__08_bloom)

target_link_libraries(05_advanced_lighting__08_bloom PRIVATE glad::glad glfw glm::glm-header-only stb::stb)

target_compile_definitions(05_advanced_lighting__08_bloom PRIVATE OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets")
