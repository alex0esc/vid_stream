#pragma once
#include "asio.hpp"
#include <cstdint>
#include <memory>
using asio::ip::tcp;

//only for gcc and gnu
#define PACKED __attribute__((packed))

namespace vsa {
  
  enum struct PacketType {
    UserInformation,
    ChatMessage
  };
  
  
  struct PACKED PacketHeader {
    uint16_t m_type = -1;
    uint16_t m_size = 0; 
  };
  
  
  class Packet : public std::enable_shared_from_this<Packet> {
    using SendHandler = std::function<void(bool)>;
    using RecieveHandler = std::function<void(std::shared_ptr<Packet>)>;
    
    PacketHeader m_header;
    size_t m_memory_size = 0;
  public:
    void* m_memory = nullptr;
    
    Packet() = default;
    Packet(PacketType type, size_t memory_size);
    ~Packet();
    Packet(const Packet&) = delete;
    Packet& operator=(const Packet&) = delete;  
    
    void static asyncReceive(tcp::socket& socket, RecieveHandler handler);
    void asyncSend(tcp::socket& socket, SendHandler handler);
  
    void setSize(size_t size);
    size_t getSize();
    void setMemorySize(size_t memory_size);
    PacketType getType();
    void setType(PacketType type);
  };
  
  
  class PacketManager : public std::enable_shared_from_this<PacketManager> {
    using DisconnectHandler = std::function<void()>;
    using PacketHandler = std::function<void(std::shared_ptr<Packet>)>;
    
    tcp::socket& m_socket;
    bool m_recieveing = false;
    DisconnectHandler m_disconnect_handler = nullptr;
    PacketHandler m_packet_handler = nullptr;
    
    void doReceive();
  public:
    PacketManager() = delete;
    PacketManager(tcp::socket& socket);
    PacketManager(const PacketManager&) = delete;
    PacketManager& operator=(const PacketManager&) = delete; 
    
    void startReceive();
    void stopReceive();
    void setPacketHandler(PacketHandler hanlder);
    void setDisconnectHandler(DisconnectHandler handler);
    void sendPacket(std::shared_ptr<Packet> packet);
  };
  
}
