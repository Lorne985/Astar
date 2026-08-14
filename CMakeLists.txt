cmake_minimum_required(VERSION 3.20)
project(astar_visualizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/raylib/CMakeLists.txt")
  message(FATAL_ERROR
    "raylib submodule is missing. Run: git submodule update --init --recursive"
  )
endif()

add_subdirectory(external/raylib)

add_executable(astar_visualizer
  src/main.cpp
)

if(MSVC)
  target_compile_options(astar_visualizer PRIVATE /utf-8)
endif()

target_link_libraries(astar_visualizer PRIVATE raylib)
