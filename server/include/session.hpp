#pragma once
#include "packet.hpp"
#include <memory>

namespace vsa {
  
  class Server;
  
  class Session : public std::enable_shared_from_this<Session>  {
    tcp::socket m_socket;
    Server* m_server = nullptr;
    std::shared_ptr<Packet> m_write_packet = nullptr;
    std::shared_ptr<PacketManager> m_packet_manager = nullptr;

    std::string m_user_name = std::string();

    void handlePacket(std::shared_ptr<Packet> packet);
    void onDisconnect();
    
  public:
    Session() = delete;
    Session(asio::io_context& context, Server* server);
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;  
    
    void onConnect(); 
    tcp::socket& getSocket();
    std::shared_ptr<PacketManager> getPacketManager();
  };
  
}
