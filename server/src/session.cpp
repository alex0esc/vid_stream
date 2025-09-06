#include "session.hpp"
#include "logger.hpp"
#include "packet.hpp"
#include "server.hpp"
#include <cstring>
#include <filesystem>
#include "server_util.hpp"


namespace vsa {
  
  Session::Session(asio::io_context& context, Server* server)
    : m_socket(tcp::socket(context)), m_server(server) { 
  }
  
  void Session::onPacketReceive(std::shared_ptr<Packet> packet) {
    if(packet->getType() != PacketType::UserInformation && m_user_name.empty()) {
      LOG_ERROR("Client was kicked because it did not provied a user name");
      disconnect();
      return;
    }
    if(packet->getType() != PacketType::FileDataEnd && packet->isEmpty()) {
      LOG_ERROR("Cannot handle empty packet of type " << packet->getType().asInt() << ".");
      disconnect();
      return;
    }
    switch(packet->getType().asInt()) {
    case PacketType::UserInformation: {
      m_user_name = std::string(static_cast<char*>(packet->getMemory()), packet->getSize());
      m_chat_packet->cpyMemory(m_user_name.data(), m_user_name.length());
      m_chat_packet->cpyMemoryOffset(": ", 2, m_user_name.length());
      std::string file_names = getFileList();
      auto file_list_packet = std::make_shared<Packet>(PacketType::FileUploadList);
      file_list_packet->setSize(file_names.length());
      file_list_packet->cpyMemory(file_names.data(), file_names.length());
      m_packet_manager->queuePacket(file_list_packet);
    } break;
      
    case PacketType::ChatMessage: {
      if(m_chat_packet->isQueued())
        return;
      sendChatMessage(std::string_view(static_cast<char*>(packet->getMemory()), packet->getSize()));
    } break;
      
    case PacketType::FileDataHeader: {   
      if(m_file.is_open()) {
        LOG_ERROR("Client kicked because it tried to start two downloads or uploads at once.");
        disconnect();
        return;
      } 
      m_file_edit = true;
      m_filename = std::string(static_cast<char*>(packet->getMemory()), packet->getSize()); 
      auto path = newFilePath(m_filename);
      m_file = std::fstream(path, std::ios::binary | std::ios::trunc | std::ios::out);
      sendChatMessage("\uf15b " + m_filename);
      LOG_INFO("Upload for file " << m_filename << " was started by.");
    } break;
      
    case PacketType::FileDataChunk: {  
      m_file.write(static_cast<char*>(packet->getMemory()), packet->getSize());
    } break;
      
    case PacketType::FileDataEnd: {
      m_file.close();
      m_file_edit = false;
      m_server->updateFileList();
      LOG_INFO("Finished upload for file " << m_filename << ".");
      m_filename.clear();
    } break;
    
    case PacketType::FileDownloadRequest: {
      if(m_file.is_open()) {
        LOG_ERROR("Client tried to download although file is already loading.");
        disconnect();
        return;
      }  
      std::string_view filename = std::string_view(static_cast<char*>(packet->getMemory()), packet->getSize());
      auto filepath = getFilePath(filename);
      if(!filepath.has_value()) {
        LOG_WARN("Requested file " << filename << " does not exist and can not be downloaded.");
        return;
      }
      if(m_server->isFileEdited(filename)) {
        LOG_WARN("Requested file " << filename << " is currently beeing edited.");
        return;
      }
      m_filename = filename;
      m_file = std::fstream(filepath.value(), std::ios::binary | std::ios::ate | std::ios::in);
      m_file_size = m_file.tellg();
      m_file.seekg(0);
      auto file_header_packet = std::make_shared<Packet>(PacketType::FileDataHeader);
      file_header_packet->setSize(m_filename.length() + 8);
      *static_cast<size_t*>(file_header_packet->getMemory()) = m_file_size;
      file_header_packet->cpyMemoryOffset(m_filename.data(), m_filename.length(), 8);
      m_packet_manager->queuePacket(file_header_packet);
      LOG_INFO("Download for file " << m_filename << " has started.");
    } break;

    case PacketType::FileDeleteRequest: {
      std::string filename = std::string(static_cast<char*>(packet->getMemory()), packet->getSize());
      auto filepath = getFilePath(filename);
      if(!filepath.has_value()) {
        LOG_WARN("Requested file " << filename << " does not exist and can not be deleted.");
        return;
      } 
      if(m_server->isFileOpened(filename)) {
        LOG_WARN("Could not delete " << m_filename << " because it is currently opened.");
        return;
      }
      std::filesystem::remove(filepath.value());
      sendChatMessage("\uf00d " + filename);
      m_server->updateFileList();
      LOG_INFO("Deleted " << filename << " from uploaded files.");
    } break;

    case PacketType::UpdateWriteRate: {
      if(packet->getSize() != 8) {
        LOG_ERROR("Client sent packet of inacceptable size.");
        disconnect();
        return;
      }
      m_packet_manager->setWriteBitRate(*static_cast<size_t*>(packet->getMemory()));
    } break;
    
    default:
      LOG_WARN("Can not handle packet type " << packet->getType().asInt() << ".");
      break; 
    } 
  }
  
  void Session::onPacketSend(std::shared_ptr<Packet> packet) {
    if(packet->getType() != PacketType::FileDataHeader && packet->getType() != PacketType::FileDataChunk)
      return;    
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
  }

  void Session::onConnect() {
    m_server->m_sessiones.push_back(shared_from_this());
    LOG_INFO("New client connected (" << m_server->m_sessiones.size() << ").");
    
    m_packet_manager = std::make_shared<PacketManager>(m_server->getContext(), m_socket);
    m_packet_manager->startReceive();
    m_packet_manager->setReceiveHandler(std::bind(&Session::onPacketReceive, shared_from_this(), std::placeholders::_1));  
    m_packet_manager->setSendHandler(std::bind(&Session::onPacketSend, shared_from_this(), std::placeholders::_1));
    m_packet_manager->setDisconnectHandler(std::bind(&Session::onDisconnect, shared_from_this()));
    m_packet_manager->setWriteBitRate(1000'000'000);

    m_chat_packet = std::make_shared<Packet>(PacketType::ChatMessage);
    m_chat_packet->setReservedSize(1056);
    m_file_packet = std::make_shared<Packet>(PacketType::FileDataChunk);
    m_file_packet->setReservedSize(64000);
  }
  
  void Session::onDisconnect() {
    m_server->m_sessiones.remove(shared_from_this());
    LOG_INFO("Client disconnected (" << m_server->m_sessiones.size() << ").");
    m_file.close();
  }

  void Session::sendChatMessage(const std::string_view& message) {
    m_chat_packet->setSize(m_user_name.length() + message.length() + 2);
    m_chat_packet->cpyMemoryOffset(message.data(), message.length(), m_user_name.length() + 2);
    m_server->broadcastPacket(m_chat_packet);
  }

  void Session::connect() {
    onConnect();
  }

  void Session::disconnect() {
    m_packet_manager->stopReceive();
    m_socket.close();
    onDisconnect();
  }
  
  std::shared_ptr<PacketManager> Session::getPacketManager() {
    return m_packet_manager;
  }

  tcp::socket& Session::getSocket() {
    return m_socket;
  }
}
