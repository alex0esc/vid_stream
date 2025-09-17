#include "session.hpp"
#include "server.hpp"
#include "logger.hpp"
#include "server_util.hpp"
#include <string>

namespace  vsa {
    
  
void Session::onPacketReceive(std::shared_ptr<Packet> packet) {
    if(packet->getType() != PacketType::UserInfo && !m_registered) {
      disconnect("Kicked because client is not registered.");
      return;
    }
    if(packet->getType() != PacketType::FileDataEnd && packet->isEmpty()) {
      disconnect("Cannot handle empty packet of type " + std::to_string(packet->getType().asInt()) + ".");
      return;
    }
    switch(packet->getType().asInt()) {
    case PacketType::UserInfo: {
      std::string_view user_data = packet->asString();
      std::string_view password = std::string_view(user_data.data(), user_data.find_first_of('\0'));
      if(!m_server->isPasswordCorrect(password)) {
        disconnect("Password is wrong, no connection allowed.");
        return;
      }
      m_registered = true;
      m_username = user_data.substr(password.length() + 1);
      m_chat_packet->setString(m_username);
      m_chat_packet->setStringOffset(std::string_view(": ", 2), m_chat_packet->getSize());
      std::string filenames = getFileList();
      auto file_list_packet = std::make_shared<Packet>(PacketType::FileUploadList);
      file_list_packet->setString(filenames);
      m_packet_manager->queuePacket(file_list_packet);
    } break;
      
    case PacketType::MessageChat: {
      if(m_chat_packet->isQueued())
        return;
      broadcastChatMessage(packet->asString());
    } break;
      
    case PacketType::FileDataHeader: {   
      if(m_file.is_open()) {
        disconnect("Download or upload is already in progress.");
        return;
      } 
      m_file_edit = true;
      m_filename = packet->asString(); 
      auto path = newFilePath(m_filename);
      m_file = std::fstream(path, std::ios::binary | std::ios::trunc | std::ios::out);
      broadcastChatMessage("\uf15b " + m_filename);
      LOG_INFO("Upload for file " << m_filename << " was started by " << m_username << ".");
    } break;
      
    case PacketType::FileDataChunk: {  
      m_file.write(static_cast<char*>(packet->getMemory()), packet->getSize());
    } break;
      
    case PacketType::FileDataEnd: {
      m_file.close();
      m_file_edit = false;
      broadcastFileList();
      LOG_INFO("Finished upload for file " << m_filename << ".");
      m_filename.clear();
    } break;
    
    case PacketType::FileDownloadRequest: {
      if(m_file.is_open()) {
        disconnect("Download or upload is already in progress.");
        return;
      }  
      std::string_view filename = packet->asString();
      auto filepath = getFilePath(filename);
      if(!filepath.has_value()) {
        sendInfoMessage("Requested file does not exist and can not be downloaded.");
        return;
      }
      if(m_server->isFileEdited(filename)) {
        sendInfoMessage("Requested file is currently beeing edited.");
        return;
      }
      m_file = std::fstream(filepath.value(), std::ios::binary | std::ios::ate | std::ios::in);
      m_filename = filename;
      m_file_size = m_file.tellg();
      m_file.seekg(0);
      auto file_header_packet = std::make_shared<Packet>(PacketType::FileDataHeader);
      file_header_packet->setStringOffset(m_filename, 8);
      *static_cast<size_t*>(file_header_packet->getMemory()) = m_file_size;
      m_packet_manager->queuePacket(file_header_packet);
      LOG_INFO("Download for file " << m_filename << " has started.");
    } break;

    case PacketType::FileDeleteRequest: {
      std::string filename = std::string(packet->asString());
      auto filepath = getFilePath(filename);
      if(!filepath.has_value()) {
        sendInfoMessage("Requested file does not exist and can not be deleted.");
        return;
      } 
      if(m_server->isFileOpened(filename)) {
        sendInfoMessage("Could not delete file because it is currently opened.");
        return;
      }
      std::filesystem::remove(filepath.value());
      broadcastChatMessage("\uf00d " + filename);
      broadcastFileList();
      LOG_INFO("Deleted " << filename << " from uploaded files.");
    } break;

    case PacketType::UpdateWriteRate: {
      if(packet->getSize() != 8) {
        disconnect("UpdateWriteRate packet can only be 8 bytes in size.");
        return;
      }
      m_packet_manager->setWriteBitRate(*static_cast<size_t*>(packet->getMemory()));
    } break;
    
    default:
      disconnect("Packet type " + std::to_string(packet->getType().asInt()) + " is not expected by the server.");
      break; 
    } 
  }
}
