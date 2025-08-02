#pragma once
#include "uif_context.hpp"
#include "uif_window.hpp"


namespace uif {

  class AppBase {
  protected:
    Window m_window;
    VulkanContext m_context;
    
    float m_frame_time = 0.01;
    float m_min_frame_time = 0.01;
    
    virtual void update() = 0;
    virtual void imguiLayoutSetup() = 0;
    virtual void manageFrameTime();
        
  public:
    virtual void init();
    virtual void run();
    virtual void destroy();
    
    AppBase(const AppBase& other) = delete;  
    AppBase operator=(const AppBase& other) = delete;
    AppBase() = default;
  };
  
}
