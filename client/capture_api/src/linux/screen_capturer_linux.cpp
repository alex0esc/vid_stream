#include "linux/screen_capturer_linux.hpp"

namespace vsa {
  
  void CapturerLinux::init() {
    
  }
  
  bool CapturerLinux::captureFrame() {
    return true;
  }

  uint8_t* CapturerLinux::getFrame() {
    return nullptr;
  }

  void CapturerLinux::destory() {
    
  }
}
