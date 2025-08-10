#pragma once
#include <iostream>
#include <mutex>

#ifdef _OPTION_LOG_ERROR
  #define LOG_ERROR(msg) (slog::g_logger.begin_msg(slog::LogType::Error) << msg << std::endl).end_msg()
#else
  #define LOG_ERROR(msg)
#endif

#ifdef _OPTION_LOG_WARN
  #define LOG_WARN(msg) (slog::g_logger.begin_msg(slog::LogType::Warn) << msg << std::endl).end_msg()
#else
  #define LOG_WARN(msg)
#endif

#ifdef _OPTION_LOG_INFO
  #define LOG_INFO(msg) (slog::g_logger.begin_msg(slog::LogType::Info) << msg << std::endl).end_msg()
#else
  #define LOG_INFO(msg)
#endif

#ifdef _OPTION_LOG_TRACE
#define LOG_TRACE(msg) (slog::g_logger.begin_msg(slog::LogType::Trace) << msg << std::endl).end_msg()
#else
  #define LOG_TRACE(msg)
#endif


namespace slog {

  enum class LogType {
    Error,
    Warn,
    Info,
    Trace
  };

  class Logger {
    std::mutex m_mtx;
    std::ostream* m_out;
    bool m_use_color = true;
     
  public:  
    Logger(std::ostream& stream = std::cout) 
      : m_mtx(), m_out(&stream) {}
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void setOutStream(std::ostream* out);
    void useColor(bool use_color);
    Logger& begin_msg(LogType type);
    void end_msg();
    
    template<typename T>
    Logger& operator<<(const T& value) {
      *m_out << value;
      return *this;
    }
    
    Logger& operator<<(std::ostream& (*manip)(std::ostream&)) {
      *m_out << manip;
      return *this;
    }
  };

  extern Logger g_logger;  
  
  
  //custom buffer to read into a string
  class StringBuffer : public std::streambuf {
    std::string& m_output;

  public:
    explicit StringBuffer(std::string& output) : m_output(output) {}

  protected:
    int overflow(int character) override;    
    
  };

  class StringStream : public std::ostream {
    StringBuffer m_buffer;

  public:
    explicit StringStream(std::string& output);
    
  };
}
