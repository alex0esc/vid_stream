#pragma once
#include <iostream>

namespace slog {
  
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
