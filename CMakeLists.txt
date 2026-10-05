cmake_minimum_required(VERSION 3.16)

project(MiniGame VERSION 1.1.0 LANGUAGES CXX)

option(MINIGAME_BUILD_TESTS "Build MiniGame tests" ON)
option(MINIGAME_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(MINIGAME_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(minigame_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(MINIGAME_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(MINIGAME_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

add_library(minigame_core
    src/character.cpp
    src/game_engine.cpp
    src/player.cpp
    src/progression.cpp
    src/random.cpp
)

target_include_directories(minigame_core PUBLIC include)
minigame_enable_warnings(minigame_core)

if(MINIGAME_ENABLE_SANITIZERS AND NOT MSVC AND NOT EMSCRIPTEN)
    target_compile_options(minigame_core PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(minigame_core PRIVATE -fsanitize=address,undefined)
endif()

if(EMSCRIPTEN)
    set(MINIGAME_WEB_DIST "${CMAKE_BINARY_DIR}/web-dist")

    add_executable(minigame_web src/web_bindings.cpp)
    target_link_libraries(minigame_web PRIVATE minigame_core)
    minigame_enable_warnings(minigame_web)

    set_target_properties(minigame_web PROPERTIES
        OUTPUT_NAME "minigame"
        SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${MINIGAME_WEB_DIST}"
    )

    target_link_options(minigame_web PRIVATE
        --bind
        -sWASM=1
        -sMODULARIZE=1
        -sEXPORT_NAME=createMiniGameModule
        -sALLOW_MEMORY_GROWTH=1
        -sASSERTIONS=1
    )

    add_custom_command(TARGET minigame_web POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${PROJECT_SOURCE_DIR}/web"
                "${MINIGAME_WEB_DIST}"
        COMMENT "Copying MiniGame web frontend"
    )
else()
    add_executable(minigame
        src/main.cpp
        src/game_state.cpp
    )
    target_link_libraries(minigame PRIVATE minigame_core)
    minigame_enable_warnings(minigame)

    if(MINIGAME_ENABLE_SANITIZERS AND NOT MSVC)
        target_compile_options(minigame PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(minigame PRIVATE -fsanitize=address,undefined)
    endif()

    install(TARGETS minigame RUNTIME DESTINATION bin)
endif()

if(MINIGAME_BUILD_TESTS AND NOT EMSCRIPTEN)
    enable_testing()

    add_executable(minigame_tests tests/test_core.cpp)
    target_link_libraries(minigame_tests PRIVATE minigame_core)
    minigame_enable_warnings(minigame_tests)

    if(MINIGAME_ENABLE_SANITIZERS AND NOT MSVC)
        target_compile_options(minigame_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(minigame_tests PRIVATE -fsanitize=address,undefined)
    endif()

    add_test(NAME minigame_core_tests COMMAND minigame_tests)
endif()
