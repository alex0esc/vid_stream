#pragma once
#include "packet.hpp"
#include "uif_app_base.hpp"
#include "logger.hpp"

using asio::ip::tcp;

namespace vsa {
  
  class Client : public uif::AppBase {
    asio::io_context m_asio_context;
    tcp::resolver m_resolver = tcp::resolver(m_asio_context);
    tcp::socket m_socket = tcp::socket(m_asio_context);
    std::shared_ptr<Packet> m_write_packet = nullptr;
    std::shared_ptr<PacketManager> m_packet_manager = nullptr;
    char m_host[40] = {};
    char m_port[20] = {};
    char m_user_name[30] = {};    

    std::string m_chat = std::string();
    std::string m_log_data = std::string();
    slog::StringStream m_log_stream = slog::StringStream(m_log_data);
    
    void imguiLayoutSetup() override;
    void update() override;
    void connect();
    void onConnect();
    void onDisconnect();
    void onPacketReceive(std::shared_ptr<Packet> packet);
    
  public:
    void init() override;    
  };
}
