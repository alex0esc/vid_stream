#pragma once
#include <fstream>
#include <iostream>
#include <mutex>
#include <filesystem>


namespace slog {
  namespace fs = std::filesystem;

  class DuplicationBuffer : public std::streambuf {
    std::streambuf* m_original_buffer;
    std::ofstream m_file_buffer;

  public:
    DuplicationBuffer(std::ostream& stream, const fs::path& filepath);
    
  protected:
    virtual int overflow(int character) override;
    virtual int sync() override;
  };
  

  class Logger {
    std::mutex m_mtx = std::mutex();
    std::ostream* m_out = &std::cout;
    std::unique_ptr<DuplicationBuffer> m_duplication_buffer;
     
  public:  
    Logger() = default;  
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void addOutputFile(const fs::path& filepath);
    void setOutputStream(std::ostream* stream);
    
    Logger& begin_msg();
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

  
}
