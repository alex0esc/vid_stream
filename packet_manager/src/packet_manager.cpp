#include "packet_manager.hpp"
#include "logger.hpp"
#include <chrono>
using namespace std::chrono;


namespace vsa {

  PacketManager::PacketManager(asio::io_context& context, tcp::socket& socket)
  : m_socket(socket), m_packet_timer(context) {}
  

  void PacketManager::doReceive() {      
    auto self = shared_from_this();
    Packet::asyncReceive(m_socket, [this, self](std::shared_ptr<Packet> packet){
      if(m_recieveing == false) {
       LOG_TRACE("Stopped recieving packages, stopReceive has been called.");
       return;
      }                    
      if(packet == nullptr) {
        m_disconnect_handler();
        return;
      }
      m_receive_handler(packet);
      doReceive();
    });
  }
  
  void PacketManager::startReceive() {
    if(!m_socket.is_open()) {
      LOG_ERROR("Socket of PacketManager is closed, could not start recieving packages."); 
      return;
    }
    m_recieveing = true;
    doReceive();
  }

  void PacketManager::stopReceive() {
    m_recieveing = false;
  }
  
  void PacketManager::doSend() {
    auto self = shared_from_this();

    //limit bitrate
    if(steady_clock::now() >= m_next_upload_check) {
      m_next_wait_time -= milliseconds(10);
      m_packet_timer.expires_after(m_next_wait_time);
      m_packet_timer.async_wait([this, self](const asio::error_code& error) {
        if(error) {
          LOG_ERROR("Error inside packet timer.");
          m_sending = false;
          return;
        }
        m_next_wait_time = milliseconds(0); 
        m_next_upload_check = steady_clock::now() + milliseconds(10);                          
        doSend();
      });
      return;
    }
    
    //send packets     
    std::shared_ptr<Packet> packet = m_packet_queue.front();  
    m_packet_queue.pop_front();  
    m_next_wait_time += nanoseconds(packet->getBandwidthSize() * 8'000'000'000 / m_write_bit_rate);
    
    packet->asyncSend(m_socket, [this, self, packet](bool successful) {
      if(!successful) { 
        stopReceive();
        m_disconnect_handler();
      } else {
        m_send_handler(packet);
        if(!m_packet_queue.empty()) {
          doSend();
        } else {
          m_sending = false;                        
        }
      }
    });    
  }

  void PacketManager::queuePacket(std::shared_ptr<Packet> packet) {
    if(!m_socket.is_open()) {
      LOG_ERROR("Socket of PacketManager is closed, could not send packet."); 
      return;
    }
    auto self = shared_from_this();
    asio::dispatch([this, self, packet]() {
      m_packet_queue.push_back(packet);
      if(!m_sending) {
        m_sending = true;
        m_next_wait_time = milliseconds(0);
        m_next_upload_check = steady_clock::now() + milliseconds(10); 
        doSend();
      }             
    });
  }
  
  void PacketManager::setReceiveHandler(PacketHandler handler) {
    m_receive_handler = handler;
  }

  void PacketManager::setSendHandler(PacketHandler handler) {
    m_send_handler = handler;
  }

  void PacketManager::setDisconnectHandler(DisconnectHandler handler) {
    m_disconnect_handler = handler; 
  }

  void PacketManager::setMbitWriteRate(uint32_t rate) {
    m_write_bit_rate = rate * 1000 * 1000;
  }
}
