#pragma once
#include "logger_impl.hpp"

namespace slog {
  
  extern Logger g_logger;

  constexpr const char* getFilename(const char* path_str) {
    const char* file_str = path_str;
    while (*path_str != '\0') {
      file_str = (*path_str == '/' || *path_str == '\\') ? path_str + 1 : file_str;
      path_str++;
    }
    return file_str;
  }
}


#ifdef _OPTION_LOG_LOCATION
  #define LOG_LOCATION slog::getFilename(__FILE__) << " " << __LINE__ << ": "  
#else
  #define LOG_LOCATION
#endif


#ifdef _OPTION_LOG_ERROR
  #ifdef _OPTION_LOG_COLOR
    #define LOG_ERROR(msg) (slog::g_logger.begin_msg() << "[\033[31mError\033[0m] " << LOG_LOCATION << msg << std::endl).end_msg()
  #else
    #define LOG_ERROR(msg) (slog::g_logger.begin_msg() << "[Error] " << LOG_LOCATION << msg << std::endl).end_msg()
  #endif
#else
  #define LOG_ERROR(msg)
#endif


#ifdef _OPTION_LOG_WARN
  #ifdef _OPTION_LOG_COLOR
    #define LOG_WARN(msg) (slog::g_logger.begin_msg() << "[\033[33mWarn\033[0m] " << LOG_LOCATION << msg << std::endl).end_msg()
  #else
    #define LOG_WARN(msg) (slog::g_logger.begin_msg() << "[Warn] " << LOG_LOCATION << msg << std::endl).end_msg()
  #endif
#else
  #define LOG_WARN(msg)
#endif


#ifdef _OPTION_LOG_INFO
  #ifdef _OPTION_LOG_COLOR
    #define LOG_INFO(msg) (slog::g_logger.begin_msg() << "[\033[36mInfo\033[0m] " << LOG_LOCATION << msg << std::endl).end_msg()
  #else
    #define LOG_INFO(msg) (slog::g_logger.begin_msg() << "[Info] " << LOG_LOCATION << msg << std::endl).end_msg()
  #endif
#else
  #define LOG_INFO(msg)
#endif


#ifdef _OPTION_LOG_TRACE
  #ifdef _OPTION_LOG_COLOR
    #define LOG_TRACE(msg) (slog::g_logger.begin_msg() << "[\033[35mTrace\033[0m] " << LOG_LOCATION << msg << std::endl).end_msg()
  #else
    #define LOG_TRACE(msg) (slog::g_logger.begin_msg() << "[Trace] " << LOG_LOCATION << msg << std::endl).end_msg()
  #endif
#else
  #define LOG_TRACE(msg)
#endif

