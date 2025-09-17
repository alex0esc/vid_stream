#pragma once
#include "session.hpp"
#include "server_util.hpp"
#include <list>

namespace vsa {

  class Server {
    friend class Session;

    asio::io_context m_asio_context;
    tcp::acceptor m_acceptor;    
    std::list<std::shared_ptr<Session>> m_sessiones;

    Config m_config = getDefaultConfig();

    void handleConnections();

    void broadcastPacket(std::shared_ptr<Packet> packet);
    bool isFileOpened(const std::string_view& filename);
    bool isFileEdited(const std::string_view& filename);
    bool isPasswordCorrect(const std::string_view& password);

  public:
    Server() = delete;
    Server(uint16_t port);
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
      
    void init();  
    void run();
    void shutdown();
  };
  
}
