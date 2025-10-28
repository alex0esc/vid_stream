#include "linux/screen_capturer_linux.hpp"
#include "logger.hpp"

namespace vsa {

  int CapturerLinux::getDisplayCount() {
    return 0;
  }
  
  void CapturerLinux::init(int display) {
    m_display =  XOpenDisplay(nullptr);
    if(!m_display) {
      LOG_ERROR("Could not open display.");
      std::abort();
    }
    if(!XShmQueryExtension(m_display)) {
      LOG_ERROR("Could not find xshm extension for display.");      
      std::abort();
    }
    m_root_window = XDefaultRootWindow(m_display);
  }
  
  bool CapturerLinux::captureFrame() {
    return true;
  }

  uint8_t* CapturerLinux::getFrame() {
    return nullptr;
  }

  void CapturerLinux::destory() {
    XCloseDisplay(m_display);
  }
}
