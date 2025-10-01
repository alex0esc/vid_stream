#include "client.hpp"
#include "imgui_internal.h"
#include "logger.hpp"
#include "client_util.hpp"

extern "C" {
  #include <libavdevice/avdevice.h>
}

namespace vsa {
  
  void Client::onDragDrop(int count, const char* paths[]) {
    if(count > 1) {
      LOG_ERROR("Cant drag more than one file at a time.");
      return;
    }
    ImGuiWindow* window = ImGui::GetCurrentContext()->HoveredWindow;
    if(window == nullptr) 
      return;
    std::string name = window->Name;
    if(!name.starts_with("Files")) 
      return;
    if(!isConnected()) {
      LOG_ERROR("Could not send packet because there is no connection.");
      return;
    }
    if(isLoadingFile()) {
      LOG_ERROR("Need to wait for download or upload to finish.");
      return;
    }
    std::filesystem::path path = std::filesystem::path(paths[0]);
    m_file = std::fstream(path, std::ios::binary | std::ios::ate | std::ios::in);
    if(!m_file.is_open()) {
      LOG_ERROR("Failed to open the file which was dragged.");
      return;
    }      
    m_file_size = m_file.tellg();
    m_file.seekg(0);
    auto file_header_packet = std::make_shared<Packet>(PacketType::FileDataHeader);
    file_header_packet->setString(path.filename().string());
    m_packet_manager->queuePacket(file_header_packet);
  }



  
  void list_devices() {
    avdevice_register_all();
    void* opaque = nullptr;
    const AVInputFormat* ifmt = nullptr;

    while ((ifmt = av_input_audio_device_next(ifmt))) {
        LOG_INFO("Audio input device: " << ifmt->name);
    }
    while ((ifmt = av_input_video_device_next(ifmt))) {
        LOG_INFO("Video input device: " << ifmt->name);
    }
  }


  

  void Client::init() {
    slog::g_logger.useColor(false);
    static slog::StringStream s_log_stream = slog::StringStream(m_log);
    slog::g_logger.setOutStream(&s_log_stream);
    
    createDownloadDirectory();

    list_devices(); 
    
    AppBase::init();    
    m_window.setTitle("Vid Stream");      
    m_window.setDropCallback(std::bind(&Client::onDragDrop, this, std::placeholders::_1, std::placeholders::_2));

    m_chat.reserve(10240);
    m_log.reserve(10240);

    m_chat_packet = std::make_shared<Packet>(PacketType::MessageChat);
    m_chat_packet->setReservedSize(c_max_chat_length);
    m_file_packet = std::make_shared<Packet>(PacketType::FileDataChunk);
    m_file_packet->setReservedSize(64000);
    
    LOG_INFO("Loading fonts...");
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("assets/Cousine-Regular.ttf", 18.0);
    static const ImWchar icons_ranges[] = {0xf000, 0xf3ff, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;  
    io.Fonts->AddFontFromFileTTF("assets/fa-solid-900.ttf", 17.0, &icons_config, icons_ranges);
    io.Fonts->Build();

    LOG_INFO("Loading config...");
    loadConfig(m_config);
    
    m_asio_thread = std::thread([this]() {
      m_asio_context.run();
    });
  }   
  
  
  void Client::destroy() {
    disconnect();
    m_work_guard.reset();
    m_asio_context.stop();
    m_asio_thread.join();
    LOG_INFO("Saving config...");
    saveConfig(m_config);
    uif::AppBase::destroy();
  }
}
