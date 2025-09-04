#pragma once
#include "packet_manager.hpp"
#include "packet.hpp"
#include <memory>
#include <fstream>


namespace vsa {
  
  class Server;
  
  class Session : public std::enable_shared_from_this<Session>  {
    friend class Server;
    Server* m_server = nullptr;
    
    tcp::socket m_socket;
    std::shared_ptr<PacketManager> m_packet_manager = nullptr;
    
    std::string m_user_name = std::string();
    std::shared_ptr<Packet> m_chat_packet = nullptr;
    
    std::fstream m_file;
    bool m_file_edit = false;
    std::string m_filename = std::string();
    std::streamsize m_file_size = -1;
    std::shared_ptr<Packet> m_file_packet;

    void onPacketReceive(std::shared_ptr<Packet> packet);
    void onPacketSend(std::shared_ptr<Packet> packet);
    void onConnect();
    void onDisconnect();
    
  public:
    Session() = delete;
    Session(asio::io_context& context, Server* server);
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;  
    
    void connect();
    void disconnect();
  
    tcp::socket& getSocket();
    std::shared_ptr<PacketManager> getPacketManager();
  };
  
}
