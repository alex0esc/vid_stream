#pragma once
#include <cstdint>

namespace vsa {

  class Capturer {

  public:
    virtual void init() = 0;
    virtual bool captureFrame() = 0;    
    virtual uint8_t* getFrame() = 0;
    virtual void destory() = 0;
  };
  
}
