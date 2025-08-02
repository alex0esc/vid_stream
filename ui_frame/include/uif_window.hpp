#pragma once
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vulkan/vulkan.hpp>

namespace uif {

  class Window {
  public:
    GLFWwindow* m_window;
    vk::SurfaceKHR m_surface;
    
    Window() = default;
    Window(const Window& other) = delete;  
    Window& operator=(const Window& other) = delete;    

    
    void createWindow(std::string title);
    void setTitle(std::string title);
    void setAttribute(int attribute, bool value);
    void setSize(uint32_t width, uint32_t height);
    void setFullscreenBorderless(bool fullscreen, uint32_t monitor_index);
   
    
    void createSurface(vk::Instance instance);
    
    static void initGlfw();
    std::pair<int, int> getFrameBufferSize();
    bool shouldClose();
    bool minimized();
    void destory();
  };
  
}
