#include "server.hpp"
#include "logger.hpp"
#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <timeapi.h>
#endif

namespace vsa {

  
  void Server::handleConnections() {
    LOG_INFO("Listening for connections.");
    auto session = std::make_shared<Session>(m_asio_context, this);
    m_acceptor.async_accept(session->getSocket(), [this, session](const std::error_code& error) {
      handleConnections();
      if (error) {
        LOG_ERROR("Connection with client failed.");
        return;
      }
      session->onConnect();
      m_sessiones.push_back(session);
      LOG_INFO("New client connected, number of clients is now " << m_sessiones.size() << ".");
    });
  }  

  void Server::broadcastPacket(std::shared_ptr<Packet> packet) {
    for(const std::shared_ptr<Session>& session : m_sessiones) 
      session->getPacketManager()->queuePacket(packet);         
  }

  asio::io_context& Server::getContext() {
    return m_asio_context;
  }
  
  void Server::init() {
    #ifdef _WIN32
      timeBeginPeriod(4);
    #endif
    handleConnections();
  }  

  void Server::run() {
    
    m_asio_context.run();
  }

  void Server::shutdown() {
    #ifdef _WIN32
      timeEndPeriod(4);
    #endif
  }  
}
