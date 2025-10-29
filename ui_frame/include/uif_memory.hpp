#include "uif_context.hpp"
#include <vulkan/vulkan.hpp>

namespace uif {

  struct MemoryConfig {
    vk::DeviceSize m_size = 0; 
    vk::MemoryPropertyFlags m_properties; 
    vk::BufferUsageFlags m_usage;
  };
  
  class VulkanMemory {
    MemoryConfig m_config;
    VulkanContext* m_context = nullptr;
    
    bool m_staging = false;
    vk::Buffer m_staging_buffer = nullptr;
    vk::DeviceMemory m_staging_memory = nullptr;
    
  public:
    vk::Buffer m_buffer = nullptr;
    vk::DeviceMemory m_memory = nullptr;
    void* m_mapped_memory = nullptr;
  
    VulkanMemory() = default;
    VulkanMemory(const VulkanMemory& other) = delete;  
    VulkanMemory& operator=(const VulkanMemory& other) = delete;    

    void init(MemoryConfig& config, VulkanContext* context);
    void allocate();
    void allocateStaging();
    void uploadStaging(vk::CommandBuffer cmd_buffer);
    void uploadStaging(vk::CommandPool cmd_pool, vk::Queue queue);
    void map();
    void unmap();
    void destoryStaging();
    void destroy();
  };
  
}
