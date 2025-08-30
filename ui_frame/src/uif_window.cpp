#include "uif_window.hpp"
#include "GLFW/glfw3.h"
#include "logger.hpp"
#include "uif_util.hpp"

namespace uif {
  
  void glfwErrorCallback(int error, const char* description) {
    LOG_ERROR("Glfw error " << error << ": " << description); 
  }

  void Window::glfwDropCallback(GLFWwindow* window, int count, const char* paths[]) {
    auto drop_callback = static_cast<Window*>(glfwGetWindowUserPointer(window))->m_drop_callback;
    if(drop_callback != nullptr) {
      drop_callback(count, paths);
    }
  } 

  void Window::initGlfw() {
    if(!glfwInit() || !glfwVulkanSupported()) {
      LOG_ERROR("Failed to initialize GLFW with vulkan.");
      std::terminate();
    }
    glfwSetErrorCallback(glfwErrorCallback);
  }

  
  void Window::createWindow(std::string title) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    //create window
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* const vid_mode = glfwGetVideoMode(monitor);
    m_window = glfwCreateWindow(vid_mode->width / 2, vid_mode->height / 2, title.c_str(), nullptr, nullptr);
    
    //drop callback
    glfwSetWindowUserPointer(m_window, this);
    glfwSetDropCallback(m_window, glfwDropCallback);

    LOG_TRACE("GLFW window has been created.");
  }

  GLFWwindow* const Window::getGlfwWindow() {
    return m_window;
  }
  
  void Window::setDropCallback(DropCallbackHandler handler) {
    m_drop_callback = handler;
  }

  void Window::setTitle(std::string title) {
    glfwSetWindowTitle(m_window, title.c_str());
  }

  void Window::setAttribute(int attribute, bool value) {
    glfwSetWindowAttrib(m_window, attribute, value);
  }
  
  void Window::setSize(uint32_t width, uint32_t height) {
    glfwSetWindowSize(m_window, width, height);
  }

  void Window::setFullscreenBorderless(bool fullscreen, uint32_t monitor_index) {
    int monitor_count = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
    if(monitor_count <= monitor_index) {
      LOG_ERROR("There is no screen with index " << monitor_index << "."); 
      std::terminate();
    }
    int monitor_x, monitor_y = 0;
    glfwGetMonitorPos(monitors[monitor_index], &monitor_x, &monitor_y);
    const GLFWvidmode* mode = glfwGetVideoMode(monitors[monitor_index]);
    
    if(fullscreen) {
      setAttribute(GLFW_DECORATED, false);
      setAttribute(GLFW_RESIZABLE, false);
      glfwSetWindowPos(m_window, monitor_x, monitor_y);
      glfwSetWindowSize(m_window, mode->width, mode->height);
    } else {
      setAttribute(GLFW_DECORATED, true);
      setAttribute(GLFW_RESIZABLE, true);
      glfwSetWindowPos(m_window, monitor_x + mode->width / 4, monitor_y + mode->height / 4);
      glfwSetWindowSize(m_window, mode->width / 2, mode->height / 2);
    }
  }


  void Window::createSurface(vk::Instance instance) {
    VkSurfaceKHR surface;
    checkVkResult(glfwCreateWindowSurface(instance, m_window, nullptr, &surface));
    m_surface = vk::SurfaceKHR(surface);
    LOG_TRACE("VkSurfaceKHR has been created.");
  }

  vk::SurfaceKHR& Window::getSurfaceKHR() {
    return m_surface;
  }

  std::pair<int, int> Window::getFrameBufferSize() {
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    return std::pair{width, height};
  }

  bool Window::shouldClose() {
    return glfwWindowShouldClose(m_window);
  }

  bool Window::minimized() {
    return glfwGetWindowAttrib(m_window, GLFW_ICONIFIED);
  }

  void Window::destory() {
    glfwDestroyWindow(m_window);
  }
}
