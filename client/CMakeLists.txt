#project
cmake_minimum_required(VERSION 3.20)
project(vid_stream_client VERSION 1.0.0 LANGUAGES CXX)

#C++ Standart
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#source files
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

#compiltaion
add_executable(${PROJECT_NAME} ${SOURCES})

#compile as GUI app on windows
if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE YES)
    target_link_libraries(${PROJECT_NAME} PRIVATE d3d11 dxgi)
endif()

#include directories and libraries
target_link_libraries(${PROJECT_NAME} PRIVATE logger)
target_link_libraries(${PROJECT_NAME} PRIVATE ui_frame)
target_link_libraries(${PROJECT_NAME} PRIVATE packet_manager)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

