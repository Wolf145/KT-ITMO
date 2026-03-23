function(setup_target target)
    set(is_gcc OFF)
    set(is_clang OFF)
    set(is_msvc OFF)

    # Determine compiler.
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(is_gcc ON)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(is_clang ON)
    elseif(MSVC)
        set(is_msvc ON)
    endif()

    # Setup warnings.
    if(is_msvc)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    elseif(is_clang OR is_gcc)
        target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic
                                                 -Wold-style-cast -Wextra-semi
                                                 -Woverloaded-virtual
                                                 -Wzero-as-null-pointer-constant
                                                 -Wnull-dereference
        )
        if(is_clang)
            target_compile_options(${target} PRIVATE -Wimplicit-fallthrough -Wshadow-uncaptured-local -Wloop-analysis)
        else()
            target_compile_options(${target} PRIVATE -Wimplicit-fallthrough=5 -Wshadow=compatible-local -Wduplicated-branches -Wduplicated-cond -Wsuggest-override)
        endif()
    endif()

    # Setup sanitizers.
    if(BUILD_USE_SANITIZERS)
        if(is_msvc)
            target_compile_options(${target} PRIVATE /fsanitize=address)
            target_compile_definitions(${target} PRIVATE _DISABLE_STRING_ANNOTATION=1 _DISABLE_VECTOR_ANNOTATION=1)
        else()
            if(WIN32)
                target_compile_options(${target} PRIVATE -fsanitize=address)
                target_link_options(${target} PRIVATE -fsanitize=address)
            else()
                target_compile_options(${target} PRIVATE -fsanitize=address,leak,undefined)
                target_link_options(${target} PRIVATE -fsanitize=address,leak,undefined)
            endif()
            target_compile_options(${target} PRIVATE -fno-sanitize-recover=all -fno-optimize-sibling-calls -fno-omit-frame-pointer)
        endif()
    endif()

    if(WIN32)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
endfunction()
