#include "imgui_impl_glfw.h"
#include "logger.hpp"
#include "uif_context.hpp"
#include "uif_util.hpp"

namespace uif {
  

  void VulkanContext::renderFrame(ImDrawData* draw_data) {    
    vk::Semaphore image_acquired_semaphore  = m_window_data.FrameSemaphores[m_window_data.SemaphoreIndex].ImageAcquiredSemaphore;
    vk::Semaphore render_complete_semaphore = m_window_data.FrameSemaphores[m_window_data.SemaphoreIndex].RenderCompleteSemaphore;

    vk::ResultValue<uint32_t> value = m_device.acquireNextImageKHR(m_window_data.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, m_dldi);
    if (value.result == vk::Result::eErrorOutOfDateKHR || value.result == vk::Result::eSuboptimalKHR) {
        m_swapchain_rebuild = true;
        return;
    } else if (value.result != vk::Result::eSuccess) 
      LOG_ERROR("Aquireing the next image has caused an error.");
     
    m_window_data.FrameIndex = value.value;
    
    ImGui_ImplVulkanH_Frame frame_data = m_window_data.Frames[m_window_data.FrameIndex];
        
    //wait for Frame to be presented
    vk::Fence fence = vk::Fence(frame_data.Fence);
    checkVkResult(m_device.waitForFences({ fence }, true, UINT64_MAX, m_dldi));
    m_device.resetFences({ fence }, m_dldi);
    
    //reset command pool
    m_device.resetCommandPool(frame_data.CommandPool); 
    vk::CommandBuffer cmd_buffer = vk::CommandBuffer(frame_data.CommandBuffer);
    
    //begin command buffer
    vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmd_buffer.begin(begin_info, m_dldi);

    //custom buffer function
    if(m_buffer_function != nullptr)
      m_buffer_function(cmd_buffer);
    
    //begin render pass
    vk::RenderPassBeginInfo render_begin_info(
      m_window_data.RenderPass, 
      frame_data.Framebuffer, 
      vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(m_window_data.Width, m_window_data.Height)),
      1, &c_background_color);
    cmd_buffer.beginRenderPass(render_begin_info, vk::SubpassContents::eInline, m_dldi);
    
    //custom render pass function
    if(m_renderpass_function != nullptr)
      m_renderpass_function(cmd_buffer);

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, frame_data.CommandBuffer);
    
    // Submit command buffer
    cmd_buffer.endRenderPass(m_dldi);
    cmd_buffer.end(m_dldi);

    vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submit_info(1, &image_acquired_semaphore, &wait_stage, 1, &cmd_buffer, 1, &render_complete_semaphore);
    m_graphics_queue.submit({ submit_info }, fence, m_dldi);
  }  

  void VulkanContext::presentFrame() {
    if (m_swapchain_rebuild)
        return;
    vk::Semaphore render_complete_semaphore = m_window_data.FrameSemaphores[m_window_data.SemaphoreIndex].RenderCompleteSemaphore;
    vk::SwapchainKHR swapchain = m_window_data.Swapchain;
    vk::PresentInfoKHR present_info(1, &render_complete_semaphore, 1, &swapchain, &m_window_data.FrameIndex);
    vk::Result result = m_graphics_queue.presentKHR(&present_info, m_dldi);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
      m_swapchain_rebuild = true;
      return;
    }
    checkVkResult(result);
    m_window_data.SemaphoreIndex = (m_window_data.SemaphoreIndex + 1) % m_window_data.SemaphoreCount;
  }


  bool VulkanContext::newFrame() {  
    std::pair fb_size = m_window->getFrameBufferSize();
    if (fb_size.first > 0 && fb_size.second > 0 && (m_swapchain_rebuild || m_window_data.Width != fb_size.first || m_window_data.Height != fb_size.second)) {
      ImGui_ImplVulkan_SetMinImageCount(c_min_image_count);
      ImGui_ImplVulkanH_CreateOrResizeWindow(m_instance, m_device_physical, m_device, &m_window_data, m_graphics_queue_family_index, nullptr, fb_size.first, fb_size.second, c_min_image_count);
      m_window_data.FrameIndex = 0;
      m_swapchain_rebuild = false;
      LOG_TRACE("Main window resized.");
    }
    if (m_window->minimized()) {
      ImGui_ImplGlfw_Sleep(10);
      return false;
    }  
        
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    return true;
  }

  void VulkanContext::render() {    
    ImGui::Render();
    ImDrawData* main_draw_data = ImGui::GetDrawData();
    const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
    if (!main_is_minimized)
      renderFrame(main_draw_data);

    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    // Present Main Platform Window
    if (!main_is_minimized)
      presentFrame();
  }
}
