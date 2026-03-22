cmake_minimum_required(VERSION 3.24)
project(cartographer_of_unmade_places)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

# --- SDL2 ---
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.12
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2)

# --- SDL2_ttf ---
FetchContent_Declare(
    SDL2_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-2.22.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
set(SDL2TTF_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2TTF_SAMPLES OFF CACHE BOOL "" FORCE)
set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS FALSE)
FetchContent_MakeAvailable(SDL2_ttf)

set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Renderer.cpp
    src/Input.cpp
    src/Camera.cpp
    src/Player.cpp
    src/TileMap.cpp
    src/MappingSystem.cpp
    src/DialogueSystem.cpp
    src/VoidSystem.cpp
    src/NPC.cpp
    src/AssetManager.cpp
    src/ParticleSystem.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME}
    SDL2::SDL2main
    SDL2::SDL2-static
    SDL2_ttf::SDL2_ttf-static
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} winmm imm32 setupapi version)
endif()
