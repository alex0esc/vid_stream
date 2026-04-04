#pragma once
#include "screen_capturer.hpp"


namespace vsa {

  class CapturerLinux : public Capturer {
                
  public:
    std::vector<DisplayInfo> listDisplays() override;
    void init(DisplayInfo& display_info) override;
    bool captureFrame() override;
    void copyFrame(void* dst_memory) override;
    void destory() override;

  };
}
