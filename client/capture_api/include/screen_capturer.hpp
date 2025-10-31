#pragma once

#include <string>
#include <thread>
#include <vector>
namespace vsa {

  struct DisplayInfo {
    std::string m_name = std::string();
    int m_width = 0;
    int m_height = 0;
    int m_offset_x = 0;
    int m_offset_y = 0;
    int m_buffer_size = 0;
  }; 

  class Capturer {
  protected:
    std::jthread m_capture_thread;
    
  public:
    std::mutex m_copy_mutex;
    DisplayInfo m_display_info;
    
    virtual std::vector<DisplayInfo> listDisplays() = 0;
    virtual void init(DisplayInfo& display_info) = 0;
    virtual bool captureFrame() = 0;    
    virtual void copyFrame(void* dst_memory) = 0;
    void startAsyncCapture(void* dst_memory);
    void stopAsyncCapture();
    virtual void destory() = 0;

    Capturer(const Capturer& other) = delete;  
    Capturer operator=(const Capturer& other) = delete;
    Capturer() = default;
  };
  
}
