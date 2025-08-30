#include "client.hpp"
#include "logger.hpp"
#include "packet.hpp"
#include <filesystem>


namespace vsa {
  
  void Client::onConnect() {
    m_packet_manager = std::make_shared<PacketManager>(m_asio_context, m_socket);
    m_packet_manager->startReceive();
    m_packet_manager->setDisconnectHandler(std::bind(&Client::onDisconnect, this));
    m_packet_manager->setReceiveHandler(std::bind(&Client::onPacketReceive, this, std::placeholders::_1));
    m_packet_manager->setSendHandler(std::bind(&Client::onPacketSend, this, std::placeholders::_1));
    
    //send user data
    auto user_data_packet = std::make_shared<Packet>(PacketType::UserInformation, 30);
    user_data_packet->setSize(strlen(m_user_name));
    memcpy(user_data_packet->m_memory, m_user_name, user_data_packet->getSize());
    m_packet_manager->queuePacket(user_data_packet);
  }
    

  void Client::onDisconnect() {
    m_file_list.clear();
    m_file.close();     
    m_socket.close();   
    LOG_INFO("Disconnected from the server.");
  }
  

  void Client::onPacketReceive(std::shared_ptr<Packet> packet) {
    switch(packet->getType()) {

    case PacketType::ChatMessage: {
      m_chat.append(reinterpret_cast<char*>(packet->m_memory));
      m_chat.push_back('\n');
    } break;

    case PacketType::FileUploadList: {
      std::string list_string = std::string(reinterpret_cast<char*>(packet->m_memory), packet->getSize());
      std::stringstream list_stream(list_string);
      std::string file_name;
      m_file_list.clear();
      while(std::getline(list_stream, file_name, '\0'))  
        m_file_list.push_back(file_name);  
    } break;

    case PacketType::FileLoadHeader: {   
      m_file_size = *reinterpret_cast<size_t*>(packet->m_memory);
      std::string filename = std::string(reinterpret_cast<char*>(packet->m_memory) + 8, packet->getSize() - 8);
      std::filesystem::path filepath = "downloads";
      if(!std::filesystem::exists(filepath)) 
          std::filesystem::create_directory(filepath);
      
      filepath /= filename;
      m_file = std::fstream(filepath, std::ios::binary | std::ios::trunc | std::ios::out);
    } break;
      
    case PacketType::FileLoadData: {  
      if(!m_file.is_open()) {
        LOG_ERROR("Could not create or open file for received packet.");
        return;
      }
      m_file.write(reinterpret_cast<char*>(packet->m_memory), packet->getSize());
    } break;
      
    case PacketType::FileLoadEnd: {
      m_file.close();
    } break;

    default:
      LOG_ERROR("Can not handle packet of type " << packet->getTypeInt() << ".");
      break;
    }
  } 
  
  void Client::onPacketSend(std::shared_ptr<Packet> packet) {    
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


  void Client::connect() {
    if(strlen(m_user_name) == 0) {
      LOG_ERROR("Can not connect without username.");
      return;
    }
    LOG_INFO("Trying to connect to " << m_host << ":" << m_port << ".");
    m_resolver.async_resolve(m_host, m_port, [this](const asio::error_code& error, tcp::resolver::results_type results) {
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
        LOG_INFO("Successfully connected to server.");
        onConnect();                      
      });                                          
    });
  }
   

  void Client::disconnect() {
    m_socket.close();
  }     
}
