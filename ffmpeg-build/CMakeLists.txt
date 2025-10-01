find_package(PkgConfig REQUIRED)

set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_LIST_DIR}/lib/pkgconfig")
pkg_check_modules(FFMPEG REQUIRED 
  libavdevice
  libavformat
  libavutil
  libavfilter
  libavcodec
  libswscale
  libswresample)

add_library(ffmpeg INTERFACE)
target_link_directories(ffmpeg INTERFACE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(ffmpeg INTERFACE ${FFMPEG_LIBRARIES})
target_include_directories(ffmpeg INTERFACE ${FFMPEG_INCLUDE_DIRS})
target_compile_options(ffmpeg INTERFACE ${FFMPEG_CFLAGS_OTHER})
