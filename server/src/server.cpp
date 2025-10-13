#include "server.hpp"
#include "logger.hpp"
#ifdef _WIN32
#include <windows.h>
#include <timeapi.h>
#endif

namespace vsa {

  Server::Server() : m_acceptor(tcp::acceptor(m_asio_context)) {} 

  
  void Server::handleConnections() {
    auto session = std::make_shared<Session>(m_asio_context, this);
    m_acceptor.async_accept(session->getSocket(), [this, session](const std::error_code& error) {
      handleConnections();
      if (error) {
        LOG_ERROR("Connection with client failed.");
        return;
      }
      session->connect();
    });
  }  

  
  void Server::broadcastPacket(std::shared_ptr<Packet> packet) {
    for(std::shared_ptr<Session> session : m_sessiones) 
      session->getPacketManager()->queuePacket(packet);         
  }

  
  bool Server::isFileOpened(const std::string_view& filename) {
    for(std::shared_ptr<Session> session : m_sessiones) {
      if(session->m_filename == filename)
        return true;
    }
    return false;
  }
  
  
  bool Server::isFileEdited(const std::string_view& filename) {
    for(std::shared_ptr<Session> session : m_sessiones) {
      if(session->m_filename == filename && session->m_file_edit)
        return true;
    }
    return false;  
  }

  
  bool Server::isPasswordCorrect(const std::string_view& password) {
    return password == m_config["password"];
  }

  
  void Server::init() {
    slog::g_logger.addOutputFile(fs::path("config") / "server_log.txt");
    createUploadDirectory();
    
    #ifdef _WIN32
      timeBeginPeriod(4);
    #endif
    
    if(!loadConfig(m_config))
      saveConfig(m_config);
    LOG_INFO("Server config loaded.");
    
    tcp::endpoint endpoint = tcp::endpoint(tcp::v4(), std::stoul(m_config["port"]));
    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(asio::socket_base::reuse_address(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen();
    LOG_INFO("Listening for connections...");
    handleConnections();
  }  

  
  void Server::run() {
    m_asio_context.run();
  }
  

  void Server::shutdown() {  
    LOG_INFO("Server shutting down.");
    m_acceptor.close();
    m_asio_context.stop();
    #ifdef _WIN32
      timeEndPeriod(4);
    #endif
  }  
}
