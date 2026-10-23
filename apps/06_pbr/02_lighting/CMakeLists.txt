add_executable(06_pbr__02_lighting main.cpp)
opengl_lab_apply_common_options(06_pbr__02_lighting)
target_link_libraries(06_pbr__02_lighting PRIVATE glad::glad glfw glm::glm-header-only stb::stb)
target_compile_definitions(06_pbr__02_lighting PRIVATE
    OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)
