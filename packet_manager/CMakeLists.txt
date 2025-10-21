#project
cmake_minimum_required(VERSION 3.20)
project(packet_manager VERSION 1.0.0 LANGUAGES CXX)

#C++ Standart
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#compilation
file(GLOB CPPS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB HPPS "${CMAKE_CURRENT_LIST_DIR}/include/*.hpp")
add_library(${PROJECT_NAME} STATIC ${CPPS} ${HPPS})

#include
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/asio)
target_link_libraries(${PROJECT_NAME} PRIVATE logger)

