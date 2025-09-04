#include "packet.hpp"
#include <chrono>
#include <deque>


namespace vsa {

  class PacketManager : public std::enable_shared_from_this<PacketManager> {
    using DisconnectHandler = std::function<void()>;
    using PacketHandler = std::function<void(std::shared_ptr<Packet>)>;
    
    DisconnectHandler m_disconnect_handler = nullptr;
    PacketHandler m_receive_handler = nullptr;
    PacketHandler m_send_handler = nullptr;
    
    tcp::socket& m_socket;
    
    bool m_recieveing = false;
    bool m_sending = false;
    
    uint64_t m_write_bit_rate = 50'000'000;
    
    std::deque<std::shared_ptr<Packet>> m_packet_queue;
    
    asio::steady_timer m_packet_timer;
    std::chrono::steady_clock::duration m_next_wait_time;
    std::chrono::steady_clock::time_point m_next_upload_check;
    
    void doReceive();
    void doSend();
    
  public:  
    PacketManager() = delete;
    PacketManager(asio::io_context& context, tcp::socket& socket);
    PacketManager(const PacketManager&) = delete;
    PacketManager& operator=(const PacketManager&) = delete; 
    
    void startReceive();
    void stopReceive();
    
    void setReceiveHandler(PacketHandler handler);
    void setSendHandler(PacketHandler handler);
    void setDisconnectHandler(DisconnectHandler handler);
    
    void queuePacket(std::shared_ptr<Packet> packet);
    
    uint64_t getWriteBitRate();
    void setWriteBitRate(uint64_t rate);
    
    
  };
}
