#include "uif_context.hpp"
#include <vulkan/vulkan.hpp>

namespace uif {
  
  class VulkanMemory {
    VulkanContext* m_context = nullptr;
    
    bool m_staging = false;
    vk::Buffer m_staging_buffer;
    vk::DeviceMemory m_staging_memory;
    
  public:
    vk::DeviceSize m_size = -1;
    vk::Buffer m_buffer;
    vk::DeviceMemory m_memory;
    void* m_mapped_memory = nullptr;
  
    VulkanMemory() = default;
    VulkanMemory(const VulkanMemory& other) = delete;  
    VulkanMemory& operator=(const VulkanMemory& other) = delete;    

    void initVk(VulkanContext* context) {
      m_context = context;
    } 
    void allocate(
      vk::DeviceSize size, 
      vk::MemoryPropertyFlagBits properties, 
      vk::Flags<vk::BufferUsageFlagBits> usage
    );
    void allocateStaging();
    void map();
    void uploadStaging(vk::CommandBuffer cmd_buffer);
    void uploadStaging(vk::CommandPool cmd_pool, vk::Queue queue);
    void unmap();
    void destoryStaging();
    void destroy();
  };
  
}
