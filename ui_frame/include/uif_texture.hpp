#pragma once
#include "uif_context.hpp"
#include <vulkan/vulkan.hpp>

namespace uif {
  
  class VulkanTexture {
    VulkanContext* m_context = nullptr;
    
    vk::DescriptorSet m_descriptor_set = nullptr;
    vk::ImageView m_image_view = nullptr;
    vk::Image m_image = nullptr;
    vk::DeviceMemory m_image_memory = nullptr;
    vk::Sampler m_sampler = nullptr;

    uint32_t m_width = -1;
    uint32_t m_height = -1;
    vk::DeviceSize m_image_size = -1;
    
  
    VulkanTexture() = default;
    VulkanTexture(const VulkanTexture& other) = delete;  
    VulkanTexture& operator=(const VulkanTexture& other) = delete;    

  
    void initVk(VulkanContext* context) { m_context = context; }
    void allocate(uint32_t width, uint32_t height);
    VkDescriptorSet getDescriptor() { return m_descriptor_set; }
    void destroy();
  public:
  };   
  
}
