add_executable(06_pbr__03_diffuse_irradiance main.cpp)
opengl_lab_apply_common_options(06_pbr__03_diffuse_irradiance)
target_link_libraries(06_pbr__03_diffuse_irradiance PRIVATE glad::glad glfw glm::glm-header-only stb::stb)
target_compile_definitions(06_pbr__03_diffuse_irradiance PRIVATE
    OPENGL_LAB_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)
