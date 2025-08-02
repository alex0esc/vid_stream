#include "uif_app_base.hpp"
#include "uif_util.hpp"
#include <chrono>

namespace uif {

  void AppBase::init() {
    Window::initGlfw();
    m_window.createWindow("Ufi App");
    m_context.init(&m_window);        
  }  

  
  void AppBase::manageFrameTime() {
    using namespace std::chrono;
    
    static auto time_last = high_resolution_clock::now();
    m_frame_time = duration_cast<microseconds>(high_resolution_clock::now() - time_last).count() / 1000000.0;
        
    float wait_time = m_min_frame_time - m_frame_time; 
    if (wait_time > 0.0)
      sleep_for(wait_time * 1000000);
    
    
    auto time_now = high_resolution_clock::now();
    m_frame_time = duration_cast<microseconds>(time_now - time_last).count() / 1000000.0;
    time_last = time_now;
  }

  
  void AppBase::run() {
    while(!m_window.shouldClose()) {          
      manageFrameTime();
      
      glfwPollEvents();
      
      if(!m_context.newFrame())
        continue;
      
      imguiLayoutSetup();
      update();

      m_context.render();     
    }
  }
  

  void AppBase::destroy() {
    m_context.destroy();
    m_window.destory();
  }
  
}
