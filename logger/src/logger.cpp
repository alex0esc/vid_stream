#include "logger.hpp"

namespace slog {
    
  Logger g_logger = Logger();

  ///set the stream the logger should write to all its messages
  void Logger::setOutStream(std::ostream* out) {
    m_out = out;
  }

  void Logger::useColor(bool use_color) {
    m_use_color = use_color;
  }

  Logger& Logger::begin_msg(LogType type) {
    m_mtx.lock();
    switch(type) {
    case LogType::Error:
      if(m_use_color)
        return *this << "[\033[31mError\033[0m] ";
      else 
        return *this << "[Error] ";
    case LogType::Warn:
      if(m_use_color)
        return *this << "[\033[33mWarn\033[0m] ";
      else
        return *this << "[Warn] ";
    case LogType::Info:
      if(m_use_color)
        return *this << "[\033[36mInfo\033[0m] ";
      else
        return *this << "[Info] ";
    case LogType::Trace:
      if(m_use_color)
        return *this << "[\033[35mTrace\033[0m] ";
      else
        return *this << "[Trace] ";
    };
    
  }

  void Logger::end_msg() {
    m_mtx.unlock();
  }
  
  
  int StringBuffer::overflow(int character) {
    if(character != EOF) { 
      m_output.push_back(static_cast<char>(character));  
    }
    return character; 
  }   

  StringStream::StringStream(std::string& output) : std::ostream(nullptr), m_buffer(output) {
    rdbuf(&m_buffer);
  }
}

