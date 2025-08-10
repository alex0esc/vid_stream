#pragma once
#include "imgui_impl_vulkan.h"
#include "uif_window.hpp"
#include <vulkan/vulkan.hpp>

#ifndef NDEBUG
#define BUILD_DEBUG
#endif


namespace uif {
   
  class VulkanContext {
    static constexpr int c_min_image_count = 2;
    static constexpr vk::ClearValue c_background_color = { vk::ClearColorValue(0.15f, 0.15f, 0.15f, 1.0f) };
    
    void createVkInstance();  
    void createWindow();
    void chosePhysicalDevice();
    void choseDeviceQueues();
    void createLogicalDevice();
    void getQueues();
    void createDescriptorPool();
    void setupVulkanWindow();
    void setupImGUI();
        
    void renderFrame(ImDrawData* draw_data);
    void presentFrame();  
    
    #ifdef BUILD_DEBUG
    vk::DebugUtilsMessengerEXT m_debug_messenger; 
    void createDebugMessenger();
    #endif    
    
  public:
    Window* m_window;
    
    vk::Instance m_instance;
    vk::detail::DispatchLoaderDynamic m_dldi;
    vk::PhysicalDevice m_device_physical;
    vk::Device m_device;
    
    int32_t m_graphics_queue_family_index;
    vk::Queue m_graphics_queue;
    bool m_has_compute_queue = false;
    int32_t m_compute_queue_family_index;
    vk::Queue m_compute_queue;
    
    vk::DescriptorPool m_descriptor_pool;
    ImGui_ImplVulkanH_Window m_window_data;
    
    bool m_swapchain_rebuild;    
        
    VulkanContext() = default;
    VulkanContext(const VulkanContext& other) = delete;  
    VulkanContext& operator=(const VulkanContext& other) = delete;    

    
    void init(Window* window); 
    bool newFrame();
    void render();
    void destroy();
  };
    
}
