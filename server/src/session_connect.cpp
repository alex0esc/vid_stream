#include "server.hpp"
#include "logger.hpp"
#include "session.hpp"
  
namespace vsa {
  
  void Session::onConnect() {
    m_packet_manager = std::make_shared<PacketManager>(m_server->m_asio_context, m_socket);
    m_packet_manager->startReceive();
    m_packet_manager->setReceiveHandler(std::bind(&Session::onPacketReceive, shared_from_this(), std::placeholders::_1));  
    m_packet_manager->setSendHandler(std::bind(&Session::onPacketSend, shared_from_this(), std::placeholders::_1));
    m_packet_manager->setDisconnectHandler(std::bind(&Session::onDisconnect, shared_from_this()));
    m_packet_manager->setWriteBitRate(1000'000'000);

    m_chat_packet = std::make_shared<Packet>(PacketType::MessageChat);
    m_chat_packet->setReservedSize(1056);
    m_file_packet = std::make_shared<Packet>(PacketType::FileDataChunk);
    m_file_packet->setReservedSize(64000);

    m_server->m_sessiones.push_back(shared_from_this());
    LOG_INFO("New client connected (" << m_server->m_sessiones.size() << ").");
  }
  
  void Session::onDisconnect() {
    m_file.close();
    m_server->m_sessiones.remove(shared_from_this());
    LOG_INFO("Client disconnected (" << m_server->m_sessiones.size() << ").");
  }

  void Session::connect() {
    onConnect();
  }

  void Session::disconnect(const std::string_view& message) {
    if(message.empty()) {
      m_packet_manager->stopReceive();
      m_socket.close();
      onDisconnect();
      LOG_WARN("Client kicked without comment.");
    } else {
      m_disconnecting = true;
      auto disconnect_packet = std::make_shared<Packet>(PacketType::MessageInfo);
      disconnect_packet->setString(message);
      m_packet_manager->queuePacket(disconnect_packet);
      LOG_WARN("Client kicked: " << message);
    }
  }

  Session::Session(asio::io_context& context, Server* server)
    : m_socket(tcp::socket(context)), m_server(server) { 
  }

  std::shared_ptr<PacketManager> Session::getPacketManager() {
    return m_packet_manager;
  }

  tcp::socket& Session::getSocket() {
    return m_socket;
  }
}
