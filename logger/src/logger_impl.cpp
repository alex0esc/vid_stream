#include "logger_impl.hpp"
#include <memory>

namespace slog {
    
  Logger g_logger;

  void Logger::addOutputFile(const fs::path& filepath) {
    m_duplication_buffer = std::make_unique<DuplicationBuffer>(*m_out, filepath);
  }

  void Logger::setOutputStream(std::ostream* stream) {
    m_duplication_buffer.reset();
    m_out = stream;
  }

  Logger& Logger::begin_msg() {
    m_mtx.lock();
    return *this;
  }

  void Logger::end_msg() {
    m_mtx.unlock();
  }

  DuplicationBuffer::DuplicationBuffer(std::ostream& stream, const fs::path& filepath)
    : m_original_buffer(stream.rdbuf()), m_file_buffer(filepath, std::ios::app) {
    stream.rdbuf(this);   
  }

  int DuplicationBuffer::overflow(int character) {
    if(character == EOF)
      return !EOF;
    int r1 = m_original_buffer->sputc(character);
    int r2 = m_file_buffer.put(character).good() ? character : EOF;
    return (r1 == EOF || r2 == EOF) ? EOF : character;
  }

  int DuplicationBuffer::sync() {
    int r1 = m_original_buffer->pubsync();
    int r2 = m_file_buffer.flush().good() ? 0 : -1;
    return (r1 == -1 || r2 == -1) ? -1 : 0;
  }
}

