cmake_minimum_required(VERSION 3.20)
project(astar_visualizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# include(FetchContent)

# FetchContent_Declare(
#     raylib
#     GIT_REPOSITORY https://github.com/raysan5/raylib.git
#     GIT_TAG 5.5
# )

# FetchContent_MakeAvailable(raylib)

add_subdirectory(external/raylib)

add_executable(astar_visualizer
  src/main.cpp
)

target_link_libraries(astar_visualizer PRIVATE raylib)
