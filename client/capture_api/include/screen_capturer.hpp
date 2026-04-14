#pragma once
#include <memory>
#include <mutex>
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
  }; 

  class Capturer {
  protected:
    std::jthread m_capture_thread;
    
  public:
    std::mutex m_copy_mutex;
    float m_capture_fps = 60;
    
    virtual std::vector<std::unique_ptr<DisplayInfo>> listDisplays() = 0;
    virtual void init(std::unique_ptr<DisplayInfo>& display_info) = 0;
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
