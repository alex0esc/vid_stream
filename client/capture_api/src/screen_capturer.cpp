#include "screen_capturer.hpp"
#include <thread>

namespace vsa {


  void Capturer::startAsyncCapture(void* dst_memory) {
    m_capture_thread = std::jthread([this, dst_memory](std::stop_token token) {
      while(!token.stop_requested()) {
        if(captureFrame()) {
          m_copy_mutex.lock();
          copyFrame(dst_memory);
          m_copy_mutex.unlock();
        }
      }                               
    });
  }  

  void Capturer::stopAsyncCapture() {  
    m_capture_thread.request_stop();
    m_copy_mutex.unlock();
    m_capture_thread.join();
  }
  
}
