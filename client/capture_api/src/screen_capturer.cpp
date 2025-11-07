#include "screen_capturer.hpp"
#include <chrono>
#include <thread>

namespace vsa {


  void Capturer::startAsyncCapture(void* dst_memory) {
    m_capture_thread = std::jthread([this, dst_memory](std::stop_token token) {      
      while(!token.stop_requested()) {
        using namespace std::chrono;
        static auto time_last = steady_clock::now();
        float frame_time = duration_cast<microseconds>(steady_clock::now() - time_last).count() / 1000000.0;
        float wait_time = (1 / m_capture_fps) - frame_time; 
        if (wait_time > 0.0)
          std::this_thread::sleep_for(nanoseconds(static_cast<uint64_t>(wait_time * 1e+9)));
        time_last = steady_clock::now();

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
