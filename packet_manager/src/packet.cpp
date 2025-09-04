#include "packet.hpp"
#include "logger.hpp"
#include <memory>


namespace vsa {  
    
  Packet::~Packet() {
    if(m_memory != nullptr)
      delete[] m_memory;
  }

  Packet::Packet(PacketType type) {
    m_header.m_type = type.asInt();
  }

  Packet::Packet(PacketType type, size_t size) : m_memory_size(size), m_memory(new uint8_t[size]()){
    m_header.m_size = size;
    m_header.m_type = type.asInt(); 
  }
  
  void Packet::asyncSend(tcp::socket& socket, SendHandler handler) {
    if(m_header.m_size > c_max_packet_size) {
      LOG_ERROR("Can not send packet which exceeds maximum size (" << c_max_packet_size << ").");
      return;
    }
    auto self = shared_from_this();
    std::array<asio::const_buffer, 2> buffers = {
      asio::buffer(&m_header, sizeof(m_header)),
      asio::buffer(m_memory, m_header.m_size) };
    
    asio::async_write(socket, buffers, [self, handler](const asio::error_code& error, const size_t&) {
      if(error) {
        LOG_ERROR("Could not send packet of type " << self->getType().asInt() << ": " << error.message());
        handler(false);
        return;
      } 
      LOG_TRACE("Sent packet of type " << self->getType().asInt() << " with length " << self->getSize() << ".");
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
          packet->m_header.m_size << " and type is " << packet->getType().asInt() << ".");
        handler(nullptr);
        return;
      }
      if(packet->m_header.m_size > c_max_packet_size) {
        LOG_ERROR("Received packet which exceeds maximum size (" << c_max_packet_size << ").");
        handler(nullptr);
        return;
      }
      if(packet->m_header.m_size <= 0) {
        LOG_WARN("Recieved empty packet of type " << packet->getType().asInt() << ".");
        handler(packet);
        return;
      } 
      packet->setReservedSize(packet->m_header.m_size);
      asio::async_read(socket, asio::buffer(packet->m_memory, packet->m_memory_size), [packet, handler](const asio::error_code& error, const size_t&) {
        if(error) { 
          LOG_TRACE("Could not receive packet data: " << error.message());               
          handler(nullptr);
          return;
        }
        LOG_TRACE("Recieved packet of type " << packet->getType().asInt() << " and size " << packet->getSize() << ".");
        handler(packet);
      });
    });
  }
  
  bool Packet::isQueued() {
    return m_queued_count;
  }  
  
  size_t Packet::getSize() {
    return m_header.m_size;
  }
  
  void Packet::setSize(size_t size) {
    m_header.m_size = size;
    if(size > m_memory_size)
      setReservedSize(size);
  }  
  
  size_t Packet::getBandwidthSize() {
    return m_header.m_size + sizeof(PacketHeader);
  }

  size_t Packet::getReservedSize() {
    return m_memory_size;
  }
  
  void Packet::setReservedSize(size_t memory_size) {
    uint8_t* new_memory = new uint8_t[memory_size]();
    if(m_memory != nullptr) {
      memcpy(new_memory, m_memory, m_memory_size > memory_size ? memory_size : m_memory_size);
      delete[] m_memory;
    }
    m_memory_size = memory_size;
    m_memory = new_memory;
  }
  
  PacketType Packet::getType() {
    return PacketType::fromInt(m_header.m_type);
  }
  
  void Packet::setType(PacketType type) {
    m_header.m_type = type.asInt();
  }

  bool Packet::isEmpty() {
    if(getSize() <= 0 || m_memory == nullptr)
      return true;   
    return false;
  }

  void* Packet::getMemory() {
    return m_memory;
  }

  void Packet::cpyMemory(void const* source, size_t size) {
    assert(size <= m_memory_size);
    memcpy(m_memory, source, size);
  }
  
  void Packet::cpyMemoryOffset(void const* source, size_t size, size_t offset) {
    assert((offset + size) <= m_memory_size);
    memcpy(m_memory + offset, source, size);
  }
}
