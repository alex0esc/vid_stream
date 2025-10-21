#project
cmake_minimum_required(VERSION 3.20)
project(vid_stream VERSION 1.0.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

#compile command for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_custom_target(copy_compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
    COMMENT "Copying compile_commands.json to project root."
)


#create necessary directories
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/assets)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/config)

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
option(LOG_COLOR "Enable colored logging" OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    option(LOG_TRACE "Enable logging traces" OFF)
endif()



#build flags
if(MSVC)
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} /Zi /Od /W4 /WX")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od /W4 /WX")
    set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} /O2 /GL /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL /DNDEBUG")
endif()


if((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -g -O0 -Wall")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall")    
    set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} -g -O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O3 -DNDEBUG")
endif()

#submodules
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/logger)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/packet_manager)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ui_frame)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/client)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/server)


