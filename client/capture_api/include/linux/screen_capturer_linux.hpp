#pragma once
#include "screen_capturer.hpp"


namespace vsa {

  class CapturerLinux : public Capturer {
            
  public:
    void init() override;
    bool captureFrame() override;
    uint8_t* getFrame() override;
    void destory() override;

  };
}
