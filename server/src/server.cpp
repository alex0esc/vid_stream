#include "server.hpp"
#include "logger.hpp"
#include "server_util.hpp"
#ifdef _WIN32
#include <windows.h>
#include <timeapi.h>
#endif

namespace vsa {

  
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
  
  void Server::updateFileList() {
    std::string file_names = getFileList();
    auto file_list_packet = std::make_shared<Packet>(PacketType::FileUploadList);
    file_list_packet->setSize(file_names.length());
    file_list_packet->cpyMemory(file_names.data(), file_names.length());
    broadcastPacket(file_list_packet);
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

  asio::io_context& Server::getContext() {
    return m_asio_context;
  }
  
  void Server::init() {
    #ifdef _WIN32
      timeBeginPeriod(4);
    #endif
    LOG_INFO("Listening for connections.");
    createUploadDirectory();
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
