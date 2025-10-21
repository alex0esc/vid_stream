#project
cmake_minimum_required(VERSION 3.20)
project(capture_api VERSION 1.0.0 LANGUAGES CXX)

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

#platform
if (WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE d3d11 dxgi)
  file(GLOB WIN_CPPS "${CMAKE_CURRENT_LIST_DIR}/src/windows/*.cpp")
  file(GLOB WIN_HPPS "${CMAKE_CURRENT_LIST_DIR}/include/windows/*.hpp")
  target_sources(${PROJECT_NAME} PRIVATE ${WIN_CPPS} ${WIN_HPPS})
elseif(APPLE)
  #target_link_libraries(${PROJECT_NAME} PRIVATE TODO)
  file(GLOB MAC_CPPS "${CMAKE_CURRENT_LIST_DIR}/src/mac/*.cpp")
  file(GLOB MAC_HPPS "${CMAKE_CURRENT_LIST_DIR}/include/mac/*.hpp")
  target_sources(${PROJECT_NAME} PRIVATE ${MAC_CPPS} ${MAC_HPPS})
elseif(UNIX)
  #target_link_libraries(${PROJECT_NAME} PRIVATE TODO)
  file(GLOB LINUX_CPPS "${CMAKE_CURRENT_LIST_DIR}/src/linux/*.cpp")
  file(GLOB LINUX_HPPS "${CMAKE_CURRENT_LIST_DIR}/include/linux/*.hpp")
  target_sources(${PROJECT_NAME} PRIVATE ${LINUX_CPPS} ${LINUX_HPPS})
endif()

#include
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE logger)
