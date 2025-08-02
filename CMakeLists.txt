#project
cmake_minimum_required(VERSION 3.20)
project(vid_stream VERSION 1.0.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

#compile command for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#create necessary directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config)

#necessary compile definitions
add_compile_definitions(ASIO_STANDALONE)
if(WIN32)
    add_compile_definitions(
        _WIN32_WINNT=0x0A00
        WIN32_LEAN_AND_MEAN    
        NOMINMAX
    )
endif()

#options
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    option(LOG_TRACE "Enable logging traces" OFF)
endif()

#submodules
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logger)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ui_frame)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/client)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/server)

#build flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -flto")
