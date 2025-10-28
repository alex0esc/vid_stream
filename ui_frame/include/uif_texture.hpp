#pragma once
#include "uif_context.hpp"
#include <vulkan/vulkan.hpp>

namespace uif {
  
  class VulkanTexture {
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

    uint32_t m_width = -1;
    uint32_t m_height = -1;
    vk::DeviceSize m_image_size = -1;
    void* m_mapped_memory = nullptr;
    
  
    VulkanTexture() = default;
    VulkanTexture(const VulkanTexture& other) = delete;  
    VulkanTexture& operator=(const VulkanTexture& other) = delete;    

  
    void initVk(VulkanContext* context) { m_context = context; }
    void allocate(uint32_t width, uint32_t height);
    void allocateStaging();
    void uploadStaging(vk::CommandBuffer cmd_buffer);
    VkDescriptorSet getDescriptor() { return m_descriptor_set; }
    void map();
    void unmap();
    void destroyStaging();
    void destroy();
  
  };   
  
}
