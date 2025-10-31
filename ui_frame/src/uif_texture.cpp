#include "uif_texture.hpp"
#include "imgui_impl_vulkan.h"
#include "logger.hpp"
#include "uif_context.hpp"
#include "uif_util.hpp"
#include "vulkan/vulkan.hpp"


namespace uif {

  void VulkanTexture::init(TextureConfig& config, VulkanContext* context) {
    m_config = config;
    m_image_size = config.m_width * config.m_height * 4;
    m_context = context;
  }

  
  void VulkanTexture::allocate() {    
    vk::ImageCreateInfo image_info(
      vk::ImageCreateFlags(),
      vk::ImageType::e2D, 
      vk::Format::eR8G8B8A8Unorm, 
      vk::Extent3D(m_config.m_width, m_config.m_height, 1), 
      1, 1, 
      vk::SampleCountFlagBits::e1, 
      vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst,
      vk::SharingMode::eExclusive,
      {}, {},
      vk::ImageLayout::eUndefined,
      nullptr);
    m_image = m_context->m_device.createImage(image_info, nullptr, m_context->m_dldi);

    
    vk::MemoryRequirements mem_requirements = m_context->m_device.getImageMemoryRequirements(m_image);
    vk::MemoryAllocateInfo alloc_info(
      mem_requirements.size, 
      findMemoryType(m_context->m_device_physical, mem_requirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, m_context->m_dldi));
    m_image_memory = m_context->m_device.allocateMemory(alloc_info, nullptr, m_context->m_dldi);
    m_context->m_device.bindImageMemory(m_image, m_image_memory, 0);
    
    vk::ImageSubresourceRange resource_range(
      vk::ImageAspectFlagBits::eColor,
      0, 1, 0, 1);
    vk::ImageViewCreateInfo image_view_info(
      vk::ImageViewCreateFlags(), 
      m_image, 
      vk::ImageViewType::e2D, 
      vk::Format::eR8G8B8A8Unorm, 
      m_config.m_component_mapping, 
      resource_range);
    m_image_view = m_context->m_device.createImageView(image_view_info, nullptr, m_context->m_dldi);
    
    vk::SamplerCreateInfo sampler_info(
      vk::SamplerCreateFlags(),
      m_config.m_upscale_filter,
      m_config.m_upscale_filter,
      vk::SamplerMipmapMode::eNearest,
      vk::SamplerAddressMode::eClampToEdge,
      vk::SamplerAddressMode::eClampToEdge,
      vk::SamplerAddressMode::eClampToEdge,
      {}, 
      false,
      1.0f,
      false,
      vk::CompareOp::eNever,
      -1000,
      1000);
    m_sampler = m_context->m_device.createSampler(sampler_info, nullptr, m_context->m_dldi);  
    
    m_descriptor_set = ImGui_ImplVulkan_AddTexture(m_sampler, m_image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);     
    LOG_TRACE("Texture initialized.");    
  }  



  void VulkanTexture::allocateStaging() {
    vk::BufferCreateInfo buffer_info(
      vk::BufferCreateFlags(), 
      m_image_size, 
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
    LOG_TRACE("Texture staging buffer allocted.");


    vk::CommandBufferAllocateInfo alc_info(
      m_context->m_command_pool, 
      vk::CommandBufferLevel::ePrimary, 
      1);
    vk::CommandBuffer cmd_buffer;
    checkVkResult(m_context->m_device.allocateCommandBuffers(&alc_info, &cmd_buffer));
    vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd_buffer.begin(begin_info, m_context->m_dldi);
    
    vk::ImageSubresourceRange resource_range;
    resource_range.setAspectMask(vk::ImageAspectFlagBits::eColor);
    resource_range.setLevelCount(1);
    resource_range.setLayerCount(1);

    vk::ImageMemoryBarrier image_barrier(
      vk::AccessFlagBits::eNone,
      vk::AccessFlagBits::eTransferWrite,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::QueueFamilyIgnored,
      vk::QueueFamilyIgnored,
      m_image,
      resource_range);
    cmd_buffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eTopOfPipe,
      vk::PipelineStageFlagBits::eTransfer,
      vk::DependencyFlagBits::eByRegion,
      0, nullptr,
      0, nullptr,
      1, &image_barrier,
      m_context->m_dldi);


    cmd_buffer.end(m_context->m_dldi);

    vk::SubmitInfo submit_info(0, nullptr, nullptr, 1, &cmd_buffer);
    m_context->m_graphics_queue.submit(submit_info);
    m_context->m_graphics_queue.waitIdle(m_context->m_dldi);
    m_context->m_device.freeCommandBuffers(m_context->m_command_pool, 1 , &cmd_buffer, m_context->m_dldi);  \
    LOG_TRACE("Image Buffer layout transformed for staging upload.");
  }



  void VulkanTexture::uploadStaging(vk::CommandBuffer cmd_buffer) {
    vk::ImageSubresourceRange resource_range;
    resource_range.setAspectMask(vk::ImageAspectFlagBits::eColor);
    resource_range.setLevelCount(1);
    resource_range.setLayerCount(1);

    vk::ImageMemoryBarrier image_barrier(
      vk::AccessFlagBits::eShaderRead,
      vk::AccessFlagBits::eTransferWrite,
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::ImageLayout::eTransferDstOptimal,
      vk::QueueFamilyIgnored,
      vk::QueueFamilyIgnored,
      m_image,
      resource_range);
    cmd_buffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eFragmentShader,
      vk::PipelineStageFlagBits::eTransfer,
      vk::DependencyFlagBits::eByRegion,
      0, nullptr,
      0, nullptr,
      1, &image_barrier,
      m_context->m_dldi);

    vk::ImageSubresourceLayers sub_layers(
      vk::ImageAspectFlagBits::eColor,
      0, 0, 1);
    vk::BufferImageCopy region(
      0, 0, 0,
      sub_layers,
      vk::Offset3D(),
      vk::Extent3D(m_config.m_width, m_config.m_height, 1));

    cmd_buffer.copyBufferToImage(
      m_staging_buffer, 
      m_image,  
      vk::ImageLayout::eTransferDstOptimal,
      1, &region, 
      m_context->m_dldi);
    
    vk::ImageMemoryBarrier image_barrier1(
      vk::AccessFlagBits::eTransferWrite,
      vk::AccessFlagBits::eShaderRead,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eShaderReadOnlyOptimal,
      vk::QueueFamilyIgnored,
      vk::QueueFamilyIgnored,
      m_image,
      resource_range);
    cmd_buffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eFragmentShader,
      vk::DependencyFlagBits::eByRegion,
      0, nullptr,
      0, nullptr,
      1, &image_barrier1,
      m_context->m_dldi);    
  }


  void VulkanTexture::uploadStagingOnce() {            
    vk::CommandBufferAllocateInfo alc_info(
      m_context->m_command_pool, 
      vk::CommandBufferLevel::ePrimary, 
      1);
    vk::CommandBuffer cmd_buffer;
    checkVkResult(m_context->m_device.allocateCommandBuffers(&alc_info, &cmd_buffer));
    vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd_buffer.begin(begin_info, m_context->m_dldi);

    uploadStaging(cmd_buffer);

    cmd_buffer.end(m_context->m_dldi);

    vk::SubmitInfo submit_info(0, nullptr, nullptr, 1, &cmd_buffer);
    m_context->m_graphics_queue.submit(submit_info);
    m_context->m_graphics_queue.waitIdle(m_context->m_dldi);
    m_context->m_device.freeCommandBuffers(m_context->m_command_pool, 1 , &cmd_buffer, m_context->m_dldi);
  }


  void VulkanTexture::map() {
    if(m_staging)
      m_mapped_memory = m_context->m_device.mapMemory(
        m_staging_memory, 0,
        m_image_size,
        vk::MemoryMapFlags(),
        m_context->m_dldi);
    else
      m_mapped_memory = m_context->m_device.mapMemory(
        m_image_memory, 0,
        m_image_size,
        vk::MemoryMapFlags(),
        m_context->m_dldi);
  }
  
  void VulkanTexture::unmap() {
    if(m_staging)
      m_context->m_device.unmapMemory(m_staging_memory, m_context->m_dldi);
    else
      m_context->m_device.unmapMemory(m_image_memory, m_context->m_dldi);
    m_mapped_memory = nullptr;
  }


  void VulkanTexture::destroyStaging() {
    m_context->m_device.freeMemory(m_staging_memory);
    m_context->m_device.destroyBuffer(m_staging_buffer);
    m_staging = false;
  }  

  void VulkanTexture::destroy() {
    m_context->m_device.waitIdle(m_context->m_dldi);
    ImGui_ImplVulkan_RemoveTexture(m_descriptor_set);
    m_context->m_device.destroySampler(m_sampler);
    m_context->m_device.destroyImageView(m_image_view);
    m_context->m_device.freeMemory(m_image_memory);
    m_context->m_device.destroyImage(m_image);
  }
}
