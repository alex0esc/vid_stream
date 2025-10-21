#project
cmake_minimum_required(VERSION 3.20)
project(ui_frame VERSION 1.0.0 LANGUAGES CXX)

#C++ Standart
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#source files
file(GLOB HPPS "${CMAKE_CURRENT_LIST_DIR}/include/*.hpp")
file(GLOB CPPS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB IMGUI_CPPS "${CMAKE_CURRENT_LIST_DIR}/imgui/*.cpp")
list(APPEND CPPS ${IMGUI_CPPS})

#glfw
set(GLFW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui/glfw-3.4) # Set this to point to an up-to-date GLFW repo
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
add_subdirectory(${GLFW_DIR} binary_dir EXCLUDE_FROM_ALL)

#compiltaion
add_library(${PROJECT_NAME} STATIC ${CPPS} ${HPPS})

#include directories and libraries
if(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE winmm)
endif()
find_package(Vulkan REQUIRED)
set(VULKAN_LIBRARIES "glfw;Vulkan::Vulkan")
target_link_libraries(${PROJECT_NAME} PUBLIC ${VULKAN_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE logger)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/imgui)

