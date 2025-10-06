#pragma once
#include <functional>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vulkan/vulkan.hpp>

namespace uif {

  class Window {
    using DropCallbackHandler = std::function<void(int, const char*[])>;
    
    GLFWwindow* m_window = nullptr;
    vk::SurfaceKHR m_surface = nullptr;
    DropCallbackHandler m_drop_callback = nullptr;
        
  public:
    Window() = default;
    Window(const Window& other) = delete;  
    Window& operator=(const Window& other) = delete;    

    void createWindow(std::string title);
    void setTitle(std::string title);
    void setAttribute(int attribute, bool value);
    void setSize(uint32_t width, uint32_t height);
    void setFullscreenBorderless(bool fullscreen, uint32_t monitor_index);
    void setDropCallback(DropCallbackHandler);
    
    void createSurface(vk::Instance instance);
    vk::SurfaceKHR& getSurfaceKHR(); 
    GLFWwindow* getGlfwWindow();
    
    
    static void initGlfw();
    static void glfwDropCallback(GLFWwindow* window, int count, const char* path[]);
    std::pair<int, int> getFrameBufferSize();
    bool shouldClose();
    bool minimized();
    void destory();
  };
  
}
