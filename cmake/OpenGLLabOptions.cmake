function(opengl_lab_setup_options)
    option(OPENGL_LAB_BUILD_EXAMPLES "Build OpenGL learning examples" ON)
    option(OPENGL_LAB_ENABLE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
    option(OPENGL_LAB_WITH_QT "Reserve switch for future Qt examples" OFF)
    option(OPENGL_LAB_WITH_OPENCV "Reserve switch for future OpenCV examples" OFF)

    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endfunction()

function(opengl_lab_apply_common_options target_name)
    target_compile_features(${target_name} PRIVATE cxx_std_23)

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

