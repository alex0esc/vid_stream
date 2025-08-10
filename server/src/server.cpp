#include "server.hpp"
#include "logger.hpp"
#include <memory>

namespace vsa {

  
  void Server::handleConnections() {
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

  void Server::broadcast(std::shared_ptr<Packet> packet) {
    for(const std::shared_ptr<Session>& session : m_sessiones) 
      session->getPacketManager()->sendPacket(packet);         
  }
  
  void Server::init() {
    LOG_INFO("Listening for connections.");
    handleConnections();
  }  

  void Server::run() {
    m_asio_context.run();
  }

  void Server::shutdown() {
    
  }  
}
