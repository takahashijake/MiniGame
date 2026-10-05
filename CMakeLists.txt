cmake_minimum_required(VERSION 3.16)
project(MiniGame VERSION 2.0.0 LANGUAGES CXX)

option(MINIGAME_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(MINIGAME_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER "${config}" config_upper)
    set("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${CMAKE_BINARY_DIR}/bin")
endforeach()

add_library(minigame_core
    src/campaign.cpp
    src/combat.cpp
    src/enemy.cpp
    src/player.cpp
    src/random.cpp
    src/save_game.cpp
)
target_include_directories(minigame_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(minigame_core PUBLIC cxx_std_17)

add_executable(minigame src/main.cpp src/terminal_ui.cpp)
target_link_libraries(minigame PRIVATE minigame_core)

function(minigame_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
        if(MINIGAME_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
        if(MINIGAME_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

minigame_warnings(minigame_core)
minigame_warnings(minigame)

if(MINIGAME_ENABLE_SANITIZERS AND NOT MSVC)
    foreach(target minigame_core minigame)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endforeach()
endif()

enable_testing()
add_executable(minigame_tests tests/minigame_tests.cpp)
target_link_libraries(minigame_tests PRIVATE minigame_core)
minigame_warnings(minigame_tests)
if(MINIGAME_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(minigame_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(minigame_tests PRIVATE -fsanitize=address,undefined)
endif()
add_test(NAME minigame_tests COMMAND minigame_tests)

install(TARGETS minigame RUNTIME DESTINATION bin)
