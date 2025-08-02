#pragma once
#include "uif_app_base.hpp"

namespace vsc {
  
  class App : public uif::AppBase {
    void update() override;
    void imguiLayoutSetup() override;
    
  public:
    void init() override;
    
  };
}
