#include "linux/screen_capturer_linux.hpp"


namespace vsa {

  std::vector<DisplayInfo> CapturerLinux::listDisplays() {
    std::vector<DisplayInfo> display_infos;
    return display_infos;
  }
  
  void CapturerLinux::init(DisplayInfo& display_info) {

  }
  
  bool CapturerLinux::captureFrame() {
    return false;
  }

  void CapturerLinux::copyFrame(void* dst_memory) {
    
  }

 
  void CapturerLinux::destory() {
  }
}
