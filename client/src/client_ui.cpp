#include "client.hpp"
#include "client_util.hpp"


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
    
    ImGui::InputText("Ip", m_host, sizeof(m_host));
    ImGui::InputText("Port", m_port, sizeof(m_port));
    ImGui::InputText("Username", m_user_name, sizeof(m_user_name));
    if(ImGui::Button("Connect", ImVec2(130, 30)))
      connect();
    ImGui::SameLine();
    if(ImGui::Button("Disconnect", ImVec2(130, 30)))
      disconnect();    

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if(ImGui::SliderInt("Mebit write", &m_mebit_write, 5, 2000, "%d", ImGuiSliderFlags_Logarithmic) && isConnected()) 
      m_packet_manager->setWriteBitRate(m_mebit_write * 1000 * 1000);  
    
    ImGui::SliderInt("MeBit read", &m_mebit_read, 5, 2000, "%d", ImGuiSliderFlags_Logarithmic); 
    if(ImGui::IsItemDeactivatedAfterEdit() && isConnected()) 
      updateReadRate();
    
    ImGui::End();
  }

  
  void Client::chatWindowSetup() {
    ImGui::Begin("Chat");
       
    ImVec2 child_size = isLoadingFile() ?
      ImVec2(-FLT_MIN, ImGui::GetWindowHeight() - 100 - ImGui::GetCursorPosY()) :
      ImVec2(-FLT_MIN, ImGui::GetWindowHeight() - 70 - ImGui::GetCursorPosY()); 
    
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0, 1.0, 1.0, 0.1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::BeginChild("##chatArea", child_size, true);
    ImGui::TextWrapped("%s", m_chat.data());
    static size_t last_length = 0;
    if(m_chat.length() != last_length) {
      ImGui::SetScrollHereY(1.0);
      last_length = m_chat.length();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    if(isLoadingFile()) 
      ImGui::ProgressBar(static_cast<float>(m_file.tellg()) / m_file_size);
    
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60);
    size_t packet_length = strlen(static_cast<char*>(m_chat_packet->getMemory()));
    if(ImGui::Button("Send", ImVec2(150, 50)) && isConnected() && !m_chat_packet->isQueued() && packet_length > 0) {
      m_chat_packet->setSize(packet_length);
      m_packet_manager->queuePacket(m_chat_packet);
    }
    
    ImGui::SameLine();
    ImGui::InputTextMultiline("##input", static_cast<char*>(m_chat_packet->getMemory()), c_max_chat_length, ImVec2(-FLT_MIN, 50));

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
    static char search_file[256] = {};
    static std::string selected_file;
  
    ImGui::Begin("Files");     
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
    
    static std::string lower_case_search;
    if(ImGui::InputText("##search", search_file, sizeof(search_file))) {
       lower_case_search = toLowerCase(search_file);
       selected_file.clear();
    }
    ImGui::PopItemWidth();
    
    ImVec2 child_size = ImGui::GetContentRegionAvail();
    if(!selected_file.empty())
      child_size.y -= 57;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0, 1.0, 1.0, 0.1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
    ImGui::BeginChild("##fileList", child_size, true);
    static float previos_width = 0;
    for(ListFile& list_file : m_file_list) {
      if(!list_file.m_lowercase_name.starts_with(lower_case_search)
      && !list_file.m_lowercase_name.ends_with(lower_case_search))
        continue;
      
      //selectable
      ImGui::PushID(list_file.m_name.c_str());
      if(ImGui::Selectable("##row", false, ImGuiSelectableFlags_None, ImVec2(ImGui::GetContentRegionAvail().x, 0))) 
        selected_file = list_file.m_name;
      ImGui::PopID();
      
      //name text
      ImGui::SameLine();
      ImGui::SetCursorPosX(0);
      
      if(ImGui::GetWindowWidth() != previos_width || list_file.m_ellipsed_name.empty()) 
        list_file.m_ellipsed_name = getTextEllipsed(list_file.m_name, ImGui::GetWindowWidth() - 130);
        
      ImGui::TextUnformatted(list_file.m_ellipsed_name.c_str());

      //size text
      ImGui::SameLine();
      ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 125);
      
      ImGui::Text("%s", list_file.m_size.c_str());
    }
    previos_width = ImGui::GetWindowWidth();
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    
    if(!selected_file.empty()) {
      ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60);
      ImGui::Text("%s", getTextEllipsed(selected_file, ImGui::GetWindowWidth() - 15).c_str());
      if(ImGui::Button("Download", ImVec2(ImGui::GetContentRegionAvail().x / 2, 30)) && !isLoadingFile()) {
        auto download_request_packet = std::make_shared<Packet>(PacketType::FileDownloadRequest);
        download_request_packet->setSize(selected_file.length());
        download_request_packet->cpyMemory(selected_file.data(), selected_file.length());
        m_packet_manager->queuePacket(download_request_packet);
      }
      ImGui::SameLine();
      if(ImGui::Button("Delete", ImVec2(ImGui::GetContentRegionAvail().x, 30))) {
        auto delete_request_packet = std::make_shared<Packet>(PacketType::FileDeleteRequest);
        delete_request_packet->setSize(selected_file.length());
        delete_request_packet->cpyMemory(selected_file.data(), selected_file.length());
        m_packet_manager->queuePacket(delete_request_packet);
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
