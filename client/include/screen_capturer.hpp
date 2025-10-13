#pragma once
#include "uif_texture.hpp"

namespace vsa {

  class Capturer {

  public:
    virtual void init() = 0;
    virtual bool captureFrame() = 0;    
    virtual bool getVulkanFrame(uif::TextureData& texture) = 0;
    virtual void destory() = 0;
  };
  
}
