#pragma once
#include "asio.hpp"
#include "packet_type.hpp"
#include <cstdint>
#include <memory>

using asio::ip::tcp; 


namespace vsa {
  
  #ifdef _MSC_VER
  #pragma pack(push, 1)
  struct PacketHeader {
    int16_t m_type = -1;
    uint32_t m_size = 0; 
  };
  #pragma pack(pop)
  
  #elif defined(__clang__) || defined(__GNUC__) 
  struct __attribute__((packed)) PacketHeader {
    int16_t m_type = -1;
    uint32_t m_size = 0; 
  };
  
  #else
   #error "Your compiler is not supported because there is no implementation to \
   remove padding from the PacketHeader for this compiler."
  #endif

 
  class Packet : public std::enable_shared_from_this<Packet> {
    friend class PacketManager;
    
    using SendHandler = std::function<void(bool)>;
    using RecieveHandler = std::function<void(std::shared_ptr<Packet>)>;

    static constexpr size_t c_max_packet_size = 1024 * 128;

    uint32_t m_queued_count = false;
    
    PacketHeader m_header;
    size_t m_memory_size = 0;
    uint8_t* m_memory = nullptr;
  public:
    
    
    Packet() = default;
    Packet(PacketType type);
    Packet(PacketType type, size_t size);
    ~Packet();
    Packet(const Packet&) = delete;
    Packet& operator=(const Packet&) = delete;  
    
    void static asyncReceive(tcp::socket& socket, RecieveHandler handler);
    void asyncSend(tcp::socket& socket, SendHandler handler);
  
    bool isQueued();
    size_t getSize();
    void setSize(size_t size);
    size_t getBandwidthSize();
    size_t getReservedSize();
    void setReservedSize(size_t memory_size);
    PacketType getType();
    void setType(PacketType type);

    bool isEmpty();
    void* getMemory();
    std::string_view asString();
    void cpyMemory(void const* source, size_t size);
    void cpyMemoryOffset(void const* source, size_t size, size_t offset);    
    void setString(const std::string_view& string);
    void setStringOffset(const std::string_view& string, size_t offset);
  };
 
}
