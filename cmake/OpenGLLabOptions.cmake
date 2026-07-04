function(opengl_lab_setup_options)
    option(OPENGL_LAB_BUILD_EXAMPLES "Build OpenGL learning examples" ON)
    option(OPENGL_LAB_ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
    option(OPENGL_LAB_WITH_QT "Reserve switch for future Qt examples" OFF)
    option(OPENGL_LAB_WITH_OPENCV "Reserve switch for future OpenCV examples" OFF)

    set(CMAKE_CXX_STANDARD 23 PARENT_SCOPE)
    set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
    set(CMAKE_CXX_EXTENSIONS OFF PARENT_SCOPE)
    set(CMAKE_CXX_SCAN_FOR_MODULES OFF PARENT_SCOPE)

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON PARENT_SCOPE)
endfunction()

function(opengl_lab_setup_clangd_support)
    if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
        return()
    endif()

    if(NOT CMAKE_GENERATOR MATCHES "Ninja|Makefiles")
        return()
    endif()

    set(_build_compile_commands "${PROJECT_BINARY_DIR}/compile_commands.json")
    set(_source_compile_commands "${PROJECT_SOURCE_DIR}/compile_commands.json")

    add_custom_command(
        OUTPUT "${_source_compile_commands}"
        COMMAND
            "${CMAKE_COMMAND}" -E copy_if_different
            "${_build_compile_commands}"
            "${_source_compile_commands}"
        DEPENDS "${_build_compile_commands}"
        COMMENT "Mirroring compile_commands.json into source root for clangd"
        VERBATIM
    )

    add_custom_target(
        opengl_lab_mirror_compile_commands ALL
        DEPENDS "${_source_compile_commands}"
    )
endfunction()

function(opengl_lab_apply_common_options target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
        if(OPENGL_LAB_ENABLE_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE /WX)
        endif()
    else()
        target_compile_options(
            ${target_name}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
        )
        if(OPENGL_LAB_ENABLE_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE -Werror)
        endif()
    endif()
endfunction()

