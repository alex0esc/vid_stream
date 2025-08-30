#include "packet.hpp"
#include "logger.hpp"
#include <memory>


namespace vsa {
  
  Packet::~Packet() {
    if(m_memory != nullptr)
      delete[] reinterpret_cast<uint8_t*>(m_memory);
  }

  Packet::Packet(PacketType type) {
    m_header.m_type = static_cast<uint16_t>(type);
  }

  Packet::Packet(PacketType type, size_t memory_size)
  : m_memory_size(memory_size), m_memory(new uint8_t[memory_size]()){
     m_header.m_type = static_cast<uint16_t>(type); 
  }
  
  void Packet::asyncSend(tcp::socket& socket, SendHandler handler) {
    auto self = shared_from_this();
    std::array<asio::const_buffer, 2> buffers = {
      asio::buffer(&m_header, sizeof(m_header)),
      asio::buffer(m_memory, m_header.m_size) };
    
    asio::async_write(socket, buffers, [this, self, handler](const asio::error_code& error, const size_t&) {
      if(error) {
        LOG_TRACE("Could not send packet of type " << m_header.m_type << ": " << error.message());
        handler(false);
        return;
      } 
      LOG_TRACE("Sent packet of type " << m_header.m_type << " with length " << m_header.m_size << ".");
      handler(true);
    });  
  } 
  
  
  void Packet::asyncReceive(tcp::socket& socket, RecieveHandler handler) {
    std::shared_ptr<Packet> packet = std::make_shared<Packet>();
    asio::async_read(socket, asio::buffer(&packet->m_header, sizeof(packet->m_header)), [&socket, packet, handler](const asio::error_code& error, const size_t&) {
      if(error) { 
        LOG_TRACE("Could not receive packet header: " << error.message());
        handler(nullptr);
        return;
      }
      if(packet->m_header.m_type == -1) {
        LOG_ERROR("Invalid packet header detected, size is " <<
          packet->m_header.m_size << " and type is " << packet->m_header.m_type << ".");
        handler(nullptr);
        return;
      }
      if(packet->m_header.m_size <= 0) {
        LOG_TRACE("Recieved empty packet of type " << packet->m_header.m_type << ".");
        handler(packet);
        return;
      } 
      packet->setMemorySize(packet->m_header.m_size);
      asio::async_read(socket, asio::buffer(packet->m_memory, packet->m_memory_size), [packet, handler](const asio::error_code& error, const size_t&) {
        if(error) { 
          LOG_TRACE("Could not receive packet data: " << error.message());               
          handler(nullptr);
          return;
        }
        LOG_TRACE("Recieved packet of type " << packet->m_header.m_type << " and size " << packet->m_header.m_size << ".");
        handler(packet);
      });
    });
  }
  
  
  void Packet::setMemorySize(size_t memory_size) {
    uint8_t* new_memory = new uint8_t[memory_size]();
    if(m_memory != nullptr) {
      memcpy(new_memory, m_memory, m_memory_size > memory_size ? memory_size : m_memory_size);
      delete[] static_cast<uint8_t*>(m_memory);
    }
    m_memory_size = memory_size;
    m_memory = new_memory;
  }  
  
  void Packet::setSize(size_t size) {
    m_header.m_size = size;
    if(size > m_memory_size)
      setMemorySize(size);
  }  
  
  size_t Packet::getSize() {
    return m_header.m_size;
  }
  
  
  size_t Packet::getBandwidthSize() {
    return getSize() + sizeof(PacketHeader);
  }
  
  
  PacketType Packet::getType() {
    return static_cast<PacketType>(m_header.m_type);
  }
  
  int Packet::getTypeInt() {
    return m_header.m_type;
  }
  
  void Packet::setType(PacketType type) {
    m_header.m_type = static_cast<uint8_t>(type);
  }
}
