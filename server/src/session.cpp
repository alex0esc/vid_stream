#include "session.hpp"
#include "logger.hpp"
#include "packet.hpp"
#include "server.hpp"
#include <cstring>
#include "server_util.hpp"


namespace vsa {
  
  Session::Session(asio::io_context& context, Server* server)
    : m_socket(tcp::socket(context)), m_server(server) { 
  }
  
  void Session::onPacketReceive(std::shared_ptr<Packet> packet) {
    switch(packet->getType()) {
    case PacketType::UserInformation: {
      m_user_name = std::string(reinterpret_cast<char*>(packet->m_memory), packet->getSize());
    } break;
      
    case PacketType::ChatMessage: {
      m_chat_packet->setSize(m_user_name.length() + packet->getSize() + 2);
      memcpy(m_chat_packet->m_memory, m_user_name.data(), m_user_name.length());
      memcpy(reinterpret_cast<char*>(m_chat_packet->m_memory) + m_user_name.length(), ": ", 2);
      memcpy(reinterpret_cast<char*>(m_chat_packet->m_memory) + m_user_name.length() + 2, packet->m_memory, packet->getSize());
      m_server->broadcastPacket(m_chat_packet);
    } break;
      
    case PacketType::FileLoadHeader: {   
      auto path = newFilePath(std::string(reinterpret_cast<char*>(packet->m_memory), packet->getSize()));
      m_file = std::fstream(path, std::ios::binary | std::ios::trunc | std::ios::out);
    } break;
      
    case PacketType::FileLoadData: {  
      if(!m_file.is_open()) {
        LOG_ERROR("Could not create or open file for received packet.");
        return;
      }
      m_file.write(reinterpret_cast<char*>(packet->m_memory), packet->getSize());
    } break;
      
    case PacketType::FileLoadEnd: {
      std::string file_names = getFileList();
      auto file_list_packet = std::make_shared<Packet>(PacketType::FileUploadList);
      file_list_packet->setSize(file_names.length());
      memcpy(file_list_packet->m_memory, file_names.data(), file_names.length());
      m_server->broadcastPacket(file_list_packet);
      m_file.close();
    } break;
    
    case PacketType::FileDownloadRequest: {
      std::string filename = std::string(reinterpret_cast<char*>(packet->m_memory), packet->getSize());
      auto filepath = getFilePath(filename);
      if(!filepath.has_value()) {
        LOG_INFO("Requested file " << filename << " does not exist and cant be downloaded.");
        return;
      }
      m_file = std::fstream(filepath.value(), std::ios::binary | std::ios::ate | std::ios::in);
      m_file_size = m_file.tellg();
      m_file.seekg(0);
      auto file_header_packet = std::make_shared<Packet>(PacketType::FileLoadHeader);
      file_header_packet->setSize(filename.length() + 8);
      *static_cast<size_t*>(file_header_packet->m_memory) = m_file_size;
      memcpy(static_cast<char*>(file_header_packet->m_memory) + 8, filename.data(), filename.length());
      m_packet_manager->queuePacket(file_header_packet);
    } break;
    
    default:
      LOG_ERROR("Can not handle packet of type " << packet->getTypeInt() << ".");
      break; 
    } 
  }
  
  void Session::onPacketSend(std::shared_ptr<Packet> packet) {
    if(packet->getType() != PacketType::FileLoadData && packet->getType() != PacketType::FileLoadHeader)
      return;    
    size_t remaining = m_file_size - m_file.tellg();
    if(remaining <= 0) {
      auto file_end_packet = std::make_shared<Packet>(PacketType::FileLoadEnd);
      m_packet_manager->queuePacket(file_end_packet);
      m_file.close();
      return;
    }
    size_t send_size = 64'000;
    if(remaining < send_size) 
      send_size = remaining;
    m_file_packet->setSize(send_size);
    m_file.read((char*) m_file_packet->m_memory, send_size);
    m_packet_manager->queuePacket(m_file_packet);
  }

  void Session::onConnect() {
    m_packet_manager = std::make_shared<PacketManager>(m_server->getContext(), m_socket);
    m_packet_manager->startReceive();
    m_packet_manager->setReceiveHandler(std::bind(&Session::onPacketReceive, shared_from_this(), std::placeholders::_1));  
    m_packet_manager->setSendHandler(std::bind(&Session::onPacketSend, shared_from_this(), std::placeholders::_1));
    m_packet_manager->setDisconnectHandler(std::bind(&Session::onDisconnect, shared_from_this()));
    m_packet_manager->setMbitWriteRate(1000);

    m_chat_packet = std::make_shared<Packet>(PacketType::ChatMessage, 1024);
    m_file_packet = std::make_shared<Packet>(PacketType::FileLoadData, 64000);
    
    //send file list
    std::string file_names = getFileList();
    auto file_list_packet = std::make_shared<Packet>(PacketType::FileUploadList);
    file_list_packet->setSize(file_names.length());
    memcpy(file_list_packet->m_memory, file_names.data(), file_names.length());
    m_packet_manager->queuePacket(file_list_packet);
  }
  
  void Session::onDisconnect() {
    m_server->m_sessiones.remove(shared_from_this());
    m_socket.close();
    m_file.close();
    LOG_INFO("Client disconnected, number of clients is now " << m_server->m_sessiones.size() << ".");
  }
  
  std::shared_ptr<PacketManager> Session::getPacketManager() {
    return m_packet_manager;
  }

  tcp::socket& Session::getSocket() {
    return m_socket;
  }
}
