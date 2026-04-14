#pragma once
#include "uif_context.hpp"
#include "uif_window.hpp"
#include "vulkan/vulkan.hpp"


namespace uif {

  class AppBase {
  protected:
    Window m_window;
    VulkanContext m_vk_context;
    
    float m_frame_time = 0.01;
    float m_min_frame_time = 0.01;
    
    virtual void update() {}
    virtual void bufferFunction(vk::CommandBuffer cmd_buffer) {};
    virtual void imguiLayoutSetup();
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
