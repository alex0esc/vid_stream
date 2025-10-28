#pragma once
#include <cstdint>

namespace vsa {

  class Capturer {

  public:
    virtual int getDisplayCount() = 0;
    virtual void init(int display = 0) = 0;
    virtual bool captureFrame() = 0;    
    virtual uint8_t* getFrame() = 0;
    virtual void destory() = 0;
  };
  
}
