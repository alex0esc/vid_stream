#pragma once
#include "session.hpp"

namespace vsa {

  class Server {
    asio::io_context m_asio_context;
    tcp::acceptor m_acceptor;
    
    void handleConnections();    
    
  public:
    std::list<std::shared_ptr<Session>> m_sessiones;
    
    Server() = delete;
    Server(uint16_t port)
    : m_acceptor(tcp::acceptor(m_asio_context, tcp::endpoint(tcp::v4(), port))) {}
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;  
    
    void broadcastPacket(std::shared_ptr<Packet> message);
    void updateFileList();
    bool isFileOpened(const std::string_view& filename);
    bool isFileEdited(const std::string_view& filename);
    
    asio::io_context& getContext();
    
    void init();  
    void run();
    void shutdown();
  };
  
}
