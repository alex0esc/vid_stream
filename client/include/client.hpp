#pragma once
#include "uif_app_base.hpp"
#include "asio/executor_work_guard.hpp"
#include "packet_manager.hpp"
#include "client_util.hpp"
#include <fstream>
using asio::ip::tcp;


namespace vsa {

  class Client : public uif::AppBase {
    static constexpr size_t c_max_chat_length = 1024;

    std::string m_log = std::string();
    Config m_config = getDefaultConfig();
    
    std::thread m_asio_thread;
    asio::io_context m_asio_context;
    asio::executor_work_guard<asio::io_context::executor_type> m_work_guard
      = asio::executor_work_guard<asio::io_context::executor_type>(m_asio_context.get_executor());
        
    tcp::socket m_socket = tcp::socket(m_asio_context);
    tcp::resolver m_resolver = tcp::resolver(m_asio_context);

    int m_mebit_write = 50;
    int m_mebit_read = 50;

    std::shared_ptr<PacketManager> m_packet_manager = nullptr;    

    std::string m_chat = std::string();
    std::shared_ptr<Packet> m_chat_packet = nullptr;
    
    std::fstream m_file;
    std::streamsize m_file_size = -1;
    std::shared_ptr<Packet> m_file_packet = nullptr;         

    struct ListFile {
      std::string m_size;
      std::string m_name;
      std::string m_ellipsed_name;
      std::string m_lowercase_name;
    };
    std::vector<ListFile> m_file_list;

    void dockingSpaceSetup();
    void generalWindowSetup();
    void chatWindowSetup();
    void logWindowSetup();
    void fileWindowSetup();
    void imguiLayoutSetup() override;
        
    void onConnect();
    void onDisconnect();
    void onPacketReceive(std::shared_ptr<Packet> packet);
    void onPacketSend(std::shared_ptr<Packet> packet);
    void onDragDrop(int count, const char* paths[]);    
    bool isConnected();
    bool isLoadingFile();
    void connect();
    void disconnect();    

    void updateReadRate();

  public:
    void init() override;    
    void destroy() override;
  };
}
