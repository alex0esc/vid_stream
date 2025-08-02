#include "app.hpp"
#include "imgui.h"


namespace vsc {

  void App::init() {
    AppBase::init();
    m_window.setTitle("Vid Stream");      
  }

  
  void App::update() {
      
  }
    
  void App::imguiLayoutSetup() {
    //create docking space overlay on main screen
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowBgAlpha(0.0); 
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | 
                                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | 
                                ImGuiWindowFlags_NoNavFocus;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("MainDockSpaceWindow", nullptr, window_flags);
    
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();    
    
    //general info window
    ImGui::Begin("General");
    ImGui::Text("Frames %f", 1.0 / m_frame_time);    
    static int m_frames = 100;
    ImGui::SliderInt("FPS Limit", &m_frames, 1, 480);
    m_min_frame_time = 1.0 / m_frames;  
    ImGui::End();
  }  
}
