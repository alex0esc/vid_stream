#include "linux/screen_capturer_linux.hpp"
#include "logger.hpp"
#include "pipewire/pipewire.h"

namespace vsa {

  std::vector<DisplayInfo> CapturerLinux::listDisplays() {
    std::vector<DisplayInfo> display_infos;
    
    return display_infos;
  }
  
  void CapturerLinux::init(DisplayInfo& display_info) {
    pw_init(nullptr, nullptr);

  }
  
  bool CapturerLinux::captureFrame() {
    
  }

  void CapturerLinux::copyFrame(void* dst_memory) {
    
  }

 
  void CapturerLinux::destory() {
    pw_deinit();
  }
}
