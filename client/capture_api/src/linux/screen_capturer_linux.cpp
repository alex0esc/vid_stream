#include "linux/screen_capturer_linux.hpp"
#include "logger.hpp"
#include "screen_capturer.hpp"
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/randr.h>
#include <cstring>
#include <stop_token>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace vsa {

  std::vector<DisplayInfo> CapturerLinux::listDisplays() {
    m_display = XOpenDisplay(nullptr);
    if(m_display == nullptr) {
      LOG_ERROR("Could not open display.");
      std::abort();
    }
    m_root_window = XDefaultRootWindow(m_display);
    int event_base, event_error;
    if(!XRRQueryExtension(m_display, &event_base, &event_error)) {
      LOG_ERROR("Could not find XRR extension");
      std::abort();
    }
    m_screen_resources = XRRGetScreenResources(m_display, m_root_window);
    if(m_screen_resources == nullptr) {
      LOG_ERROR("Could not get screen resources.");
      std::abort();
    }
    std::vector<DisplayInfo> display_infos;
    for(int index = 0; index < m_screen_resources->noutput; index++) {
      XRROutputInfo* output = XRRGetOutputInfo(
        m_display,
        m_screen_resources,
        m_screen_resources->outputs[index]);
      if(output->connection != RR_Connected || !output->crtc)
        continue;
      XRRCrtcInfo* crtc = XRRGetCrtcInfo(m_display, m_screen_resources, output->crtc);
      
      display_infos.emplace_back(
        output->name,
        static_cast<int>(crtc->width),
        static_cast<int>(crtc->height),
        crtc->x,
        crtc->y,
        static_cast<int>(crtc->height * crtc->width * 4));
    }
    return display_infos;
  }
  
  void CapturerLinux::init(DisplayInfo& display_info) {
    m_display_info = display_info;
    m_screen = XDefaultScreen(m_display);
    if(!XShmQueryExtension(m_display)) {
      LOG_ERROR("Could not find xshm extension for display.");      
      std::abort();
    }    
    m_image = XShmCreateImage(m_display,
      XDefaultVisual(m_display, m_screen),
      XDefaultDepth(m_display, m_screen),
      ZPixmap,
      nullptr,
      &m_segment_info,
      display_info.m_width,
      display_info.m_height);
    if(m_image->bytes_per_line * m_image->height != m_display_info.m_buffer_size) {
      LOG_ERROR("XImage size does not match the expected size.");
      std::abort();
    }
    if(m_image->blue_mask != 0x000000FF || m_image->green_mask != 0x0000FF00 || m_image->red_mask != 0x00FF0000) {
      LOG_ERROR("Caputred image is not BGR0.");
      std::abort();
    }
    
    m_segment_info.shmid = shmget(IPC_PRIVATE, m_image->bytes_per_line * m_image->height, IPC_CREAT | 0777);
    m_segment_info.shmaddr = m_image->data = (char*) shmat(m_segment_info.shmid, 0, 0);
    m_segment_info.readOnly = False;
    
    XShmAttach(m_display, &m_segment_info);
  }
  
  bool CapturerLinux::captureFrame() {
    XShmGetImage(m_display, m_root_window, m_image, m_display_info.m_offset_x, m_display_info.m_offset_y, AllPlanes);
    if(m_image == nullptr) {
      LOG_WARN("Could not caputre X11 image.");
      return false;
    }
    return true;
  }

  void CapturerLinux::copyFrame(void* dst_memory) {
    memcpy(dst_memory, m_image->data , m_display_info.m_buffer_size);
  }

 
  void CapturerLinux::destory() {
    XShmDetach(m_display, &m_segment_info);
    XDestroyImage(m_image);
    shmdt(m_segment_info.shmaddr);
    shmctl(m_segment_info.shmid, IPC_RMID, nullptr);
    XCloseDisplay(m_display);
  }
}
