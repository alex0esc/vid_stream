#include "client.hpp"
#include "imgui.h"
#include "packet.hpp"
#include <cstring>
#include <functional>
#include <memory>
#include <winnt.h>


namespace vsa {
  
  void Client::init() {
    slog::g_logger.useColor(false);
    slog::g_logger.setOutStream(&m_log_stream);
    AppBase::init();    
    m_window.setTitle("Vid Stream");      
    m_chat.reserve(10240);
    m_write_packet = std::make_shared<Packet>(PacketType::ChatMessage, 1024); 
  }
    
  void Client::imguiLayoutSetup() {
    //create docking space overlay on main screen
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
    
    //general info window
    ImGui::Begin("General");
    ImGui::Text("Frames %f", 1.0 / m_frame_time);    
    static int m_frames = 100;
    ImGui::SliderInt("FPS Limit", &m_frames, 1, 480);
    m_min_frame_time = 1.0 / m_frames;  
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    ImGui::InputText("Ip", m_host, 40);
    ImGui::InputText("Port", m_port, 40);
    ImGui::InputText("Username", m_user_name, 30);
    if(ImGui::Button("Connect", ImVec2(100, 30)))
      connect();
    ImGui::End();
    
    //Chat window
    ImGui::Begin("Chat");
    ImGui::InputTextMultiline("##output", m_chat.data(), m_chat.length(),
      ImVec2(-FLT_MIN, ImGui::GetWindowHeight() - 70 - ImGui::GetCursorPosY()), ImGuiInputTextFlags_ReadOnly);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60);
    
    if(ImGui::Button("Send", ImVec2(150, 50)) && m_socket.is_open()) {
      m_write_packet->setSize(strlen(reinterpret_cast<char*>(m_write_packet->m_memory)) + 1);
      m_packet_manager->sendPacket(m_write_packet);
    }
    
    ImGui::SameLine();
    ImGui::InputTextMultiline("##input", (char*) m_write_packet->m_memory, 1024, ImVec2(-FLT_MIN, 50));
    ImGui::End();

    //Log window
    ImGui::Begin("Log");
    ImGui::BeginChild("##textArea", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()),
                      false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::TextWrapped("%s", m_log_data.data());
    static size_t last_length = 0;
    if(m_log_data.length() != last_length) {
      ImGui::SetScrollHereY(1.0);
      last_length = m_log_data.length();
    }
    ImGui::EndChild();
    ImGui::End();
  }  

  void Client::update() {
    m_asio_context.poll();      
  }

  void Client::connect() {
    if(strlen(m_user_name) == 0) {
      LOG_ERROR("Can not connect without username.");
      return;
    }
    LOG_INFO("Trying to connect to " << m_host << ":" << m_port << ".");
    m_resolver.async_resolve(m_host, m_port, [this](const asio::error_code& error, tcp::resolver::results_type results) {
      if(error) {
        LOG_ERROR("Could not resolve address.");
        return;
      }
      LOG_INFO("Resolved address to host.");
      asio::async_connect(m_socket, results, [this](const asio::error_code& error, const tcp::endpoint&) {
        if(error) {
          LOG_ERROR("Could not connect to server.");
          return;
        }
        LOG_INFO("Successfully connected to server.");
        onConnect();                      
      });                                          
    });
  }  

  void Client::onConnect() {
    m_packet_manager = std::make_shared<PacketManager>(m_socket);
    m_packet_manager->startReceive();
    m_packet_manager->setDisconnectHandler(std::bind(&Client::onDisconnect, this));
    m_packet_manager->setPacketHandler(std::bind(&Client::onPacketReceive, this, std::placeholders::_1));
    
    auto user_data_packet = std::make_shared<Packet>(PacketType::UserInformation, 30);
    user_data_packet->setSize(strlen(m_user_name));
    memcpy(user_data_packet->m_memory, m_user_name, user_data_packet->getSize());
    m_packet_manager->sendPacket(user_data_packet);
  }

  void Client::onDisconnect() {
    LOG_INFO("Disconnected from the server.");
    m_socket.close();
     
  }

  void Client::onPacketReceive(std::shared_ptr<Packet> packet) {
    switch(packet->getType()) {
    case PacketType::ChatMessage:
      m_chat.append(reinterpret_cast<char*>(packet->m_memory));
      m_chat.push_back('\n');
      break;
    default:
      LOG_ERROR("Can not handle packet of type " << static_cast<int>(packet->getType()) << ".");
      break;
    }
  }
}
