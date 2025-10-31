#pragma once
#include "uif_context.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan.hpp>

namespace uif {

  struct TextureConfig {
    int m_width = 0;
    int m_height = 0;
    vk::ComponentMapping m_component_mapping = {
      vk::ComponentSwizzle::eR,
      vk::ComponentSwizzle::eG,
      vk::ComponentSwizzle::eB,
      vk::ComponentSwizzle::eA };
    vk::Filter m_upscale_filter = vk::Filter::eLinear;      
  };
  
  class VulkanTexture {
    TextureConfig m_config;
    VulkanContext* m_context = nullptr;

    bool m_staging = false;
    vk::Buffer m_staging_buffer = nullptr;
    vk::DeviceMemory m_staging_memory = nullptr;

  public: 
    vk::DescriptorSet m_descriptor_set = nullptr;
    vk::ImageView m_image_view = nullptr;
    vk::Image m_image = nullptr;
    vk::DeviceMemory m_image_memory = nullptr;
    vk::Sampler m_sampler = nullptr;

    vk::DeviceSize m_image_size = -1;
    void* m_mapped_memory = nullptr;
    
  
    VulkanTexture() = default;
    VulkanTexture(const VulkanTexture& other) = delete;  
    VulkanTexture& operator=(const VulkanTexture& other) = delete;    

  
    void init(TextureConfig& config, VulkanContext* context);
    void allocate();
    void allocateStaging();
    void uploadStaging(vk::CommandBuffer cmd_buffer);
    void uploadStagingOnce();
    VkDescriptorSet getDescriptor() { return m_descriptor_set; }
    void map();
    void unmap();
    void destroyStaging();
    void destroy();
  };   
  
}
