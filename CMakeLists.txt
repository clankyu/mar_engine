cmake_minimum_required(VERSION 3.20)
project(mar_engine C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)

include_directories(src)
include_directories(vendor/SDL/include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(engine ${SOURCES})

target_link_libraries(engine PRIVATE SDL3::SDL3)

target_link_libraries(engine PRIVATE
        winmm
        imm32
        version
        setupapi
        user32
        gdi32
        shell32
        shlwapi
        onecore
)