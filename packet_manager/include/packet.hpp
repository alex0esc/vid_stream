#pragma once
#include "asio.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
using asio::ip::tcp; 


namespace vsa {
  
  enum struct PacketType {
    UserInformation,
    ChatMessage,
    FileLoadHeader,
    FileLoadData,
    FileLoadEnd,
    FileUploadList,
    FileDownloadRequest
  };
  
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
    using SendHandler = std::function<void(bool)>;
    using RecieveHandler = std::function<void(std::shared_ptr<Packet>)>;
    
    PacketHeader m_header;
    size_t m_memory_size = 0;
  public:
    void* m_memory = nullptr;
    
    Packet() = default;
    Packet(PacketType type);
    Packet(PacketType type, size_t memory_size);
    ~Packet();
    Packet(const Packet&) = delete;
    Packet& operator=(const Packet&) = delete;  
    
    void static asyncReceive(tcp::socket& socket, RecieveHandler handler);
    void asyncSend(tcp::socket& socket, SendHandler handler);
  
    void setSize(size_t size);
    size_t getSize();
    size_t getBandwidthSize();
    void setMemorySize(size_t memory_size);
    PacketType getType();
    int getTypeInt();
    void setType(PacketType type);
  };
 
}
