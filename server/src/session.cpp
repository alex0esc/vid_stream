#include "session.hpp"
#include "logger.hpp"
#include "server.hpp"

namespace vsa {
  
  
  Session::Session(asio::io_context& context, Server* server)
    : m_socket(tcp::socket(context)), m_server(server) { 
  }

  void Session::handlePacket(std::shared_ptr<Packet> packet) {
    switch(packet->getType()) {
    case PacketType::UserInformation: 
      m_user_name = std::string(reinterpret_cast<char*>(packet->m_memory), packet->getSize());
      break;
    case PacketType::ChatMessage:
      m_write_packet->setSize(m_user_name.length() + packet->getSize() + 2);
      memcpy(m_write_packet->m_memory, m_user_name.data(), m_user_name.length());
      memcpy(reinterpret_cast<char*>(m_write_packet->m_memory) + m_user_name.length(), ": ", 2);
      memcpy(reinterpret_cast<char*>(m_write_packet->m_memory) + m_user_name.length() + 2, packet->m_memory, packet->getSize());
      m_server->broadcast(m_write_packet);
      break;
    } 
  }

  void Session::onConnect() {
    m_write_packet = std::make_shared<Packet>(PacketType::ChatMessage, 1024);
    m_packet_manager = std::make_shared<PacketManager>(m_socket);
    m_packet_manager->startReceive();
    m_packet_manager->setPacketHandler(std::bind(&Session::handlePacket, shared_from_this(), std::placeholders::_1));  
    m_packet_manager->setDisconnectHandler(std::bind(&Session::onDisconnect, shared_from_this()));
  }
  
  void Session::onDisconnect() {
    m_server->m_sessiones.remove(shared_from_this());
    m_socket.close();
    LOG_INFO("Client disconnected, number of clients is now " << m_server->m_sessiones.size() << ".");
  }
  
  std::shared_ptr<PacketManager> Session::getPacketManager() {
    return m_packet_manager;
  }

  tcp::socket& Session::getSocket() {
    return m_socket;
  }
}
