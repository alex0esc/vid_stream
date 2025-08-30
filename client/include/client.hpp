#pragma once
#include "asio/executor_work_guard.hpp"
#include "packet_manager.hpp"
#include "uif_app_base.hpp"
#include <fstream>

using asio::ip::tcp;

namespace vsa {
  
  class Client : public uif::AppBase {
    std::string m_log = std::string();
    
    std::thread m_asio_thread;
    asio::io_context m_asio_context;
    asio::executor_work_guard<asio::io_context::executor_type> m_work_guard
      = asio::executor_work_guard<asio::io_context::executor_type>(m_asio_context.get_executor());
    tcp::socket m_socket = tcp::socket(m_asio_context);
    std::shared_ptr<PacketManager> m_packet_manager = nullptr;
        
    tcp::resolver m_resolver = tcp::resolver(m_asio_context);
    char m_host[40] = {};
    char m_port[20] = {};
    char m_user_name[30] = {};
    
    std::string m_chat = std::string();
    std::shared_ptr<Packet> m_chat_packet = nullptr;
    
    std::fstream m_file;
    std::streamsize m_file_size = -1;
    std::shared_ptr<Packet> m_file_packet = nullptr;         
    
    std::vector<std::string> m_file_list;
    
    void dockingSpaceSetup();
    void generalWindowSetup();
    void chatWindowSetup();
    void logWindowSetup();
    void fileWindowSetup();
    void imguiLayoutSetup() override;
    
    void connect();
    void disconnect();
    void sendFile();
    void onConnect();
    void onDisconnect();
    void onPacketReceive(std::shared_ptr<Packet> packet);
    void onPacketSend(std::shared_ptr<Packet> packet);
    void onDragDrop(int count, const char* paths[]);    
    
  public:
    void init() override;    
    void destroy() override;
  };
}
