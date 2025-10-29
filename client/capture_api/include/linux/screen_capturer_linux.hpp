#pragma once
#include "screen_capturer.hpp"
#include "X11/Xlib.h"
#include "X11/extensions/XShm.h"
#include "X11/extensions/Xrandr.h"
#include <sys/ipc.h>
#include <sys/shm.h>


namespace vsa {

  class CapturerLinux : public Capturer {
    Display* m_display = nullptr;
    Window m_root_window = {};
    XRRScreenResources* m_screen_resources = nullptr;
    int m_screen = -1;
    XShmSegmentInfo m_segment_info = {};
    XImage* m_image = nullptr;
    
            
  public:
    std::vector<DisplayInfo> listDisplays() override;
    void init(DisplayInfo& display_info) override;
    bool captureFrame() override;
    void copyFrame(void* dst_memory) override;
    void destory() override;

  };
}
