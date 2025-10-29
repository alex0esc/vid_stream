#include "uif_memory.hpp"
#include "uif_util.hpp"

namespace uif {

  void VulkanMemory::init(MemoryConfig& config, VulkanContext* context) {
    m_context = context;
    m_config = config;
  }  

  void VulkanMemory::allocate() {
    vk::BufferCreateInfo buffer_info(
      vk::BufferCreateFlags(), 
      m_config.m_size, 
      m_config.m_usage, 
      vk::SharingMode::eExclusive);
    m_buffer = m_context->m_device.createBuffer(buffer_info, nullptr);
    vk::MemoryRequirements mem_requirements2 = m_context->m_device.getBufferMemoryRequirements(m_buffer);
    vk::MemoryAllocateInfo alloc_info2(
      mem_requirements2.size, 
      findMemoryType(m_context->m_device_physical, mem_requirements2.memoryTypeBits, m_config.m_properties, m_context->m_dldi)); 
    m_memory = m_context->m_device.allocateMemory(alloc_info2, nullptr, m_context->m_dldi);
    m_context->m_device.bindBufferMemory(m_buffer, m_memory, 0, m_context->m_dldi);
  }


  void VulkanMemory::allocateStaging() {
    vk::BufferCreateInfo buffer_info(
      vk::BufferCreateFlags(), 
      m_config.m_size, 
      vk::BufferUsageFlagBits::eTransferSrc, 
      vk::SharingMode::eExclusive);
    m_staging_buffer = m_context->m_device.createBuffer(buffer_info, nullptr);
    vk::MemoryRequirements mem_requirements2 = m_context->m_device.getBufferMemoryRequirements(m_staging_buffer);
    vk::MemoryAllocateInfo alloc_info2(
      mem_requirements2.size, 
      findMemoryType(m_context->m_device_physical, mem_requirements2.memoryTypeBits, 
        vk::MemoryPropertyFlagBits::eHostCoherent, m_context->m_dldi)); 
    m_staging_memory = m_context->m_device.allocateMemory(alloc_info2, nullptr, m_context->m_dldi);
    m_context->m_device.bindBufferMemory(m_staging_buffer, m_staging_memory, 0, m_context->m_dldi);        
    m_staging = true;
  }
  
  
  void VulkanMemory::uploadStaging(vk::CommandBuffer cmd_buffer) {            
      vk::BufferCopy region(0, 0, m_config.m_size);
      cmd_buffer.copyBuffer(
        m_staging_buffer, 
        m_buffer,  
        1, &region, 
        m_context->m_dldi);
  }


  void VulkanMemory::uploadStaging(vk::CommandPool cmd_pool, vk::Queue queue) {            
    vk::CommandBufferAllocateInfo alc_info(
      cmd_pool, 
      vk::CommandBufferLevel::ePrimary, 
      1);
    vk::CommandBuffer cmd_buffer;
    checkVkResult(m_context->m_device.allocateCommandBuffers(&alc_info, &cmd_buffer));
    vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd_buffer.begin(begin_info, m_context->m_dldi);

    uploadStaging(cmd_buffer);

    cmd_buffer.end(m_context->m_dldi);

    vk::SubmitInfo submit_info(0, nullptr, nullptr, 1, &cmd_buffer);
    queue.submit(submit_info);
    queue.waitIdle(m_context->m_dldi);
    m_context->m_device.freeCommandBuffers(cmd_pool, 1 , &cmd_buffer, m_context->m_dldi);
  }
  
  
  void VulkanMemory::map() {
    if(m_staging)
      m_mapped_memory = m_context->m_device.mapMemory(m_staging_memory, 0, m_config.m_size, vk::MemoryMapFlags(), m_context->m_dldi);
    else
      m_mapped_memory = m_context->m_device.mapMemory(m_memory, 0, m_config.m_size, vk::MemoryMapFlags(), m_context->m_dldi);
  }
  
  void VulkanMemory::unmap() {
    if(m_staging)
      m_context->m_device.unmapMemory(m_staging_memory, m_context->m_dldi);
    else
      m_context->m_device.unmapMemory(m_memory, m_context->m_dldi);
    m_mapped_memory = nullptr;
  }
  
  
  void VulkanMemory::destoryStaging() {
    m_context->m_device.freeMemory(m_staging_memory);
    m_context->m_device.destroyBuffer(m_staging_buffer);
    m_staging = false;
  }    

  void VulkanMemory::destroy() {
    m_context->m_device.freeMemory(m_memory);
    m_context->m_device.destroyBuffer(m_buffer);
  }
}
