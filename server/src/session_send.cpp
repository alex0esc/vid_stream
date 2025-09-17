#include "session.hpp"
#include "server.hpp"
#include "logger.hpp"
#include "server_util.hpp"

namespace vsa {  

  void Session::onPacketSend(std::shared_ptr<Packet> packet) {
    switch(packet->getType().asInt()) {
    case PacketType::FileDataHeader: case PacketType::FileDataChunk: {
      size_t remaining = m_file_size - m_file.tellg();
      if(remaining <= 0) {
        auto file_end_packet = std::make_shared<Packet>(PacketType::FileDataEnd);
        m_packet_manager->queuePacket(file_end_packet);
        m_file.close();
        LOG_INFO("Finished downloading file " << m_filename << ".");
        m_filename.clear();
        return;
      }
      size_t chunk_size = 64'000;
      if(remaining < chunk_size) 
        chunk_size = remaining;
      m_file_packet->setSize(chunk_size);
      m_file.read(static_cast<char*>(m_file_packet->getMemory()), chunk_size);
      m_packet_manager->queuePacket(m_file_packet);    
    } break;
    case PacketType::MessageInfo: {
      if(!m_disconnecting) 
        return;
      m_packet_manager->stopReceive();
      m_socket.close();
      onDisconnect();
    } break;
    }
  }

  void Session::sendInfoMessage(const std::string_view& message) {
    auto info_packet = std::make_shared<Packet>(PacketType::MessageInfo);
    info_packet->setString(message);
    m_server->broadcastPacket(info_packet);
  }

  void Session::broadcastChatMessage(const std::string_view& message) {
    m_chat_packet->setStringOffset(message, m_username.length() + 2);
    m_chat_packet->setStringOffset("\n", m_chat_packet->getSize());
    m_server->broadcastPacket(m_chat_packet);
  }

  void Session::broadcastFileList() {
    std::string file_names = getFileList();
    auto file_list_packet = std::make_shared<Packet>(PacketType::FileUploadList);
    file_list_packet->setString(file_names);
    m_server->broadcastPacket(file_list_packet);
  }
}
