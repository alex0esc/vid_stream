#include "asio/connect.hpp"
#include "client.hpp"
#include "logger.hpp"
#include "client_util.hpp"

namespace vsa {
  
  void Client::onConnect() {
    m_packet_manager = std::make_shared<PacketManager>(m_asio_context, m_socket);
    m_packet_manager->startReceive();
    m_packet_manager->setDisconnectHandler(std::bind(&Client::onDisconnect, this));
    m_packet_manager->setReceiveHandler(std::bind(&Client::onPacketReceive, this, std::placeholders::_1));
    m_packet_manager->setSendHandler(std::bind(&Client::onPacketSend, this, std::placeholders::_1));
    
    auto user_data_packet = std::make_shared<Packet>(PacketType::UserInfo);
    user_data_packet->setReservedSize(100);
    user_data_packet->setString(m_config["password"]);
    user_data_packet->setStringOffset(std::string_view("\0", 1), user_data_packet->getSize());
    user_data_packet->setStringOffset(m_config["username"], user_data_packet->getSize());
    m_packet_manager->queuePacket(user_data_packet);

    updateWriteRate();
    updateReadRate();

    LOG_INFO("Successfully connected to server.");
  }
    

  void Client::onDisconnect() {
    m_packet_manager = nullptr;
    m_file_list.clear();
    m_file.close();        
    LOG_INFO("Disconnected from the server.");
  }
  

  void Client::onPacketReceive(std::shared_ptr<Packet> packet) {
    switch(packet->getType().asInt()) {
    case PacketType::MessageChat: { 
      m_chat.append(packet->asString());
    } break;

    case PacketType::MessageInfo: {
      m_log.append("Server>> ");
      m_log.append(packet->asString());
      m_log.push_back('\n');
    } break;

    case PacketType::FileDataHeader: {   
      m_file_size = *static_cast<size_t*>(packet->getMemory());
      std::string_view filename = std::string_view(static_cast<char*>(packet->getMemory()) + sizeof(size_t), packet->getSize() - sizeof(size_t));
      auto filepath = newFilePath(filename);
      m_file = std::fstream(filepath, std::ios::binary | std::ios::trunc | std::ios::out);
    } break;
      
    case PacketType::FileDataChunk: {  
      m_file.write(static_cast<char*>(packet->getMemory()), packet->getSize());
    } break;
      
    case PacketType::FileDataEnd: {
      m_file.close();
    } break;

    case PacketType::FileUploadList: {
      m_file_list.clear();
      char* list = static_cast<char*>(packet->getMemory());
      size_t string_start = 8;
      for(size_t i = 8; i < packet->getSize(); i++) {
        if(list[i] == '\0') {
          m_file_list.emplace_back(
            getSizeText(*reinterpret_cast<size_t*>(list + string_start - 8)),
            std::string(list + string_start, i - string_start),
            std::string(),
            toLowerCase(std::string_view(list + string_start, i - string_start)));
          string_start = i + 1 + 8;
          i += 8;
        }
      }  
    } break;

    default:
      LOG_WARN("Can not handle packet of type " << packet->getType().asInt() << ".");
      break;
    }
  } 
  
  void Client::onPacketSend(std::shared_ptr<Packet> packet) {    
    switch(packet->getType().asInt()) {
    case PacketType::FileDataHeader: case PacketType::FileDataChunk: {
      size_t remaining = m_file_size - m_file.tellg();
      if(remaining <= 0) {
        auto file_end_packet = std::make_shared<Packet>(PacketType::FileDataEnd);
        m_packet_manager->queuePacket(file_end_packet);
        m_file.close();
        return;
      }
      size_t chunk_size = 64'000;
      if(remaining < chunk_size) 
        chunk_size = remaining;
      m_file_packet->setSize(chunk_size);
      m_file.read(static_cast<char*>(m_file_packet->getMemory()), chunk_size);
      m_packet_manager->queuePacket(m_file_packet);
    } break;

    default:
      break;
    }       
  }

  bool Client::isConnected() {
    if(m_packet_manager != nullptr && m_socket.is_open())
      return true;
    return false;
  }


  bool Client::isLoadingFile() {
    if(m_file.is_open() && isConnected())
      return true;
    return false; 
  }

  void Client::connect() {
    if(isConnected())
      return;
    if(m_config["username"].length() == 0) {
      LOG_ERROR("Can not connect without username.");
      return;
    }
    LOG_INFO("Trying to connect to " << m_config["address"] << ":" << m_config["port"] << ".");
    m_resolver.async_resolve(m_config["address"], m_config["port"], [this](const asio::error_code& error, tcp::resolver::results_type results) {
      if(error) {
        LOG_ERROR("Could not resolve address.");
        return;
      }
      LOG_INFO("Resolved address to host.");
      asio::async_connect(m_socket, results, [this](const asio::error_code& error, const tcp::endpoint&) {
        if(error) {
          LOG_ERROR("Could not connect to server.");
          return;
        }
        onConnect();                      
      });                                          
    });
  }
   
  void Client::disconnect() {
    if(!isConnected()) 
      return;
    m_packet_manager->stopReceive();
    m_socket.close();
    onDisconnect();
  }     

  void Client::updateReadRate() {
    auto write_update_packet = std::make_shared<Packet>(PacketType::UpdateWriteRate);
    write_update_packet->setSize(sizeof(size_t));
    *static_cast<size_t*>(write_update_packet->getMemory()) = m_mebit_read * 1000 * 1000;
    m_packet_manager->queuePacket(write_update_packet);
  }

  void Client::updateWriteRate() {
    m_packet_manager->setWriteBitRate(m_mebit_write * 1000 * 1000);
  }
}
