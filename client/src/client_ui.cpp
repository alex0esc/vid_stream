#include "client.hpp"
#include "imgui.h"
#include "packet.hpp"
#include <memory>

namespace vsa {
  
  void Client::dockingSpaceSetup() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowBgAlpha(0.0); 
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | 
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
  }
  
  
  void Client::generalWindowSetup() {
    ImGui::Begin("General");
    ImGui::Text("Frames %f", 1.0 / m_frame_time);    
    static int m_frames = 100;
    ImGui::SliderInt("FPS Limit", &m_frames, 30, 480);
    m_min_frame_time = 1.0 / m_frames;  
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::InputText("Ip", m_host, 40);
    ImGui::InputText("Port", m_port, 20);
    ImGui::InputText("Username", m_user_name, 30);
    if(ImGui::Button("Connect", ImVec2(130, 30)))
      connect();
    ImGui::SameLine();
    if(ImGui::Button("Disconnect", ImVec2(130, 30)))
      disconnect();    

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    static int mebit = 50;
    ImGui::SliderInt("Mebit write", &mebit, 1, 1000);
    if(m_packet_manager != nullptr)
     m_packet_manager->setMbitWriteRate(mebit);
    ImGui::End();
  }

  
  void Client::chatWindowSetup() {
    ImGui::Begin("Chat");
        
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0, 1.0, 1.0, 0.1));
    ImVec2 child_size = ImVec2(-FLT_MIN, ImGui::GetWindowHeight() - 70 - ImGui::GetCursorPosY()); 
    if(m_file.is_open()) 
      child_size = ImVec2(-FLT_MIN, ImGui::GetWindowHeight() - 100 - ImGui::GetCursorPosY());
    ImGui::BeginChild("##chatArea",
        child_size,
        false,ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::TextWrapped("%s", m_chat.data());
    static size_t last_length = 0;
    if(m_chat.length() != last_length) {
      ImGui::SetScrollHereY(1.0);
      last_length = m_chat.length();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
    
    if(m_file.is_open()) {
      ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 92);
      ImGui::ProgressBar(static_cast<float>(m_file.tellg()) / m_file_size);
    }
    
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60);
    if(ImGui::Button("Send", ImVec2(150, 50)) && m_socket.is_open()) {
      m_chat_packet->setSize(strlen(reinterpret_cast<char*>(m_chat_packet->m_memory)) + 1);
      m_packet_manager->queuePacket(m_chat_packet);
    }
    
    ImGui::SameLine();
    ImGui::InputTextMultiline("##input", (char*) m_chat_packet->m_memory, 1024, ImVec2(-FLT_MIN, 50));

    ImGui::End();
  }

  
  void Client::logWindowSetup() {
    ImGui::Begin("Log");
    ImGui::BeginChild("##logArea", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                      false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::TextWrapped("%s", m_log.data());
    static size_t last_length = 0;
    if(m_log.length() != last_length) {
      ImGui::SetScrollHereY(1.0);
      last_length = m_log.length();
    }
    ImGui::EndChild();
    ImGui::End();
  }

  void Client::fileWindowSetup() {
    ImGui::Begin("Files"); 
    for(std::string filename : m_file_list) {
      if(ImGui::Selectable(filename.c_str())) {
        auto file_request_packet = std::make_shared<Packet>(PacketType::FileDownloadRequest);
        file_request_packet->setSize(filename.length());
        memcpy(file_request_packet->m_memory, filename.data(), filename.length());
        m_packet_manager->queuePacket(file_request_packet);
      }
    }
    ImGui::End();
  }
  
  void Client::imguiLayoutSetup() {
    dockingSpaceSetup();   
    generalWindowSetup();    
    chatWindowSetup();    
    logWindowSetup();
    fileWindowSetup();
  }
}
