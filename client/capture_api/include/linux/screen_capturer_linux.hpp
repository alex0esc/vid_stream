#pragma once
#include "screen_capturer.hpp"
#include "X11/Xlib.h"
#include "X11/extensions/XShm.h"
#include "X11/extensions/Xrandr.h"
#include <sys/ipc.h>
#include <sys/shm.h>


namespace vsa {

  class CapturerLinux : public Capturer {
    Display* m_display;
    Window m_root_window;
    XImage m_image;
    XShmSegmentInfo m_segment;

            
  public:
    int getDisplayCount() override;
    void init(int display = 0) override;
    bool captureFrame() override;
    uint8_t* getFrame() override;
    void destory() override;

  };
}
