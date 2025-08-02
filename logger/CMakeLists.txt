#project
cmake_minimum_required(VERSION 3.20)
project(logger VERSION 1.0.0 LANGUAGES CXX)

#C++ Standart
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#options
option(LOG_ERROR "Enable logging errors" ON)
option(LOG_WARN "Enable logging warnings" ON)
option(LOG_INFO "Enable logging info" ON)
option(LOG_TRACE "Enable logging traces" ON)

#compilation
file(GLOB CPPS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB HPPS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")
add_library(${PROJECT_NAME} STATIC ${CPPS} ${HPPS})

#include
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

#definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC
    $<$<BOOL:${LOG_ERROR}>:_OPTION_LOG_ERROR>
    $<$<BOOL:${LOG_WARN}>:_OPTION_LOG_WARN>
    $<$<BOOL:${LOG_INFO}>:_OPTION_LOG_INFO>
    $<$<BOOL:${LOG_TRACE}>:_OPTION_LOG_TRACE>
)
