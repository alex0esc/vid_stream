#include "client.hpp"
#include "imgui_internal.h"
#include "logger.hpp"
#include "client_util.hpp"
#include "string_stream.hpp"
#include "uif_texture.hpp"
#include "vulkan/vulkan.hpp"

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
  

  void Client::init() {
    static slog::StringStream s_log_stream = slog::StringStream(m_log);
    slog::g_logger.setOutputStream(&s_log_stream);
    slog::g_logger.addOutputFile(fs::path("config") / "client_log.txt");
  
    AppBase::init();    
    m_window.setTitle("Vid Stream");      
    m_window.setDropCallback(std::bind(&Client::onDragDrop, this, std::placeholders::_1, std::placeholders::_2));
        
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("assets/Cousine-Regular.ttf", 18.0);
    static const ImWchar icons_ranges[] = {0xf000, 0xf3ff, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;  
    io.Fonts->AddFontFromFileTTF("assets/fa-solid-900.ttf", 17.0, &icons_config, icons_ranges);
    io.Fonts->Build();
    LOG_INFO("Fonts loaded.");

    createDownloadDirectory();
    if(!loadConfig(m_config))
      saveConfig(m_config);
    LOG_INFO("Client config loaded.");

    m_chat.reserve(10240);
    m_log.reserve(10240);
    m_chat_packet = std::make_shared<Packet>(PacketType::MessageChat);
    m_chat_packet->setReservedSize(c_max_chat_length);
    m_file_packet = std::make_shared<Packet>(PacketType::FileDataChunk);
    m_file_packet->setReservedSize(64000);
    
    m_asio_thread = std::thread([this]() {
      m_asio_context.run();
    });

    
    std::vector<DisplayInfo> displays = m_capturer.listDisplays();
    m_capturer.init(displays[0]);
    

    uif::TextureConfig config = {m_capturer.m_display_info.m_width, m_capturer.m_display_info.m_height};
    config.m_component_mapping.setR(vk::ComponentSwizzle::eB);
    config.m_component_mapping.setB(vk::ComponentSwizzle::eR);
    config.m_component_mapping.setA(vk::ComponentSwizzle::eOne);
    m_texture.init(config, &m_vk_context);
    m_texture.allocate();
    m_texture.allocateStaging();
    m_texture.map();
    
    //m_capturer.startAsyncCapture(m_texture.m_mapped_memory);

    /*
    m_vk_context.m_buffer_function = [this](vk::CommandBuffer buffer) {
        
    };
  */
  }   

  void Client::update() {
    
    //if(m_capturer.m_copy_mutex.try_lock()) {
      m_texture.uploadStagingOnce();
      m_capturer.m_copy_mutex.unlock();
    //}
    
  }
  
  
  void Client::destroy() {
    disconnect();
    m_capturer.stopAsyncCapture();
    m_capturer.destory();
    m_work_guard.reset();
    m_asio_context.stop();
    m_asio_thread.join();
    m_texture.destroyStaging();
    m_texture.destroy();
    uif::AppBase::destroy();
    saveConfig(m_config);
    LOG_INFO("Client config saved.");
  }
}
