#include "string_stream.hpp"

namespace slog {
  
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
