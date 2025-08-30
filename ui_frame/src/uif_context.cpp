#include "uif_context.hpp"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "logger.hpp"
#include "uif_util.hpp"
#include <cstdint>
#include <vector>


namespace uif {
    
  void VulkanContext::createVkInstance() {
    vk::ApplicationInfo app_info(
      "Imgui Setup", VK_MAKE_VERSION(1, 0, 0), 
      "None", VK_MAKE_VERSION(1, 0, 0), 
      VK_API_VERSION_1_4);       
    
    std::vector<const char*> layers;
    
    uint32_t ext_count = 0;
    const char** exts = glfwGetRequiredInstanceExtensions(&ext_count);
    std::vector<const char*> extensions = std::vector(exts, exts + ext_count);
        
    #ifdef BUILD_DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);   
    layers.push_back("VK_LAYER_KHRONOS_validation");
    #endif

    vk::InstanceCreateFlags flags = vk::InstanceCreateFlags();
    #ifdef __APPLE__
    extensions.push_back("VK_KHR_portability_enumeration");
    flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
    #endif
    
    vk::InstanceCreateInfo create_info(
      flags, &app_info, 
      layers.size(), layers.data(), 
      extensions.size(), extensions.data());
    m_instance = vk::createInstance(create_info);
    LOG_TRACE("VkInstance created.");
    m_dldi = vk::detail::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);
    LOG_TRACE("VkDispatchLoaderDynamic created.");
  } 
  

  #ifdef BUILD_DEBUG
  VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallbackFunc(
    vk::DebugUtilsMessageSeverityFlagBitsEXT msg_severity, 
    vk::DebugUtilsMessageTypeFlagsEXT msg_type, 
    const vk::DebugUtilsMessengerCallbackDataEXT* callback_data, 
    void* user_data) {
    if (msg_severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
      LOG_ERROR(callback_data->pMessage);
    if (msg_severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
      LOG_WARN(callback_data->pMessage);
    if (msg_severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
      LOG_INFO(callback_data->pMessage);    
    if (msg_severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
      LOG_TRACE(callback_data->pMessage);
    (void)user_data;
    (void)msg_type;
    return false;
  }
   
  void VulkanContext::createDebugMessenger() {    
    vk::DebugUtilsMessengerCreateInfoEXT create_info(
      vk::DebugUtilsMessengerCreateFlagsEXT(), 
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError, 
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
      debugCallbackFunc);
    m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(create_info, nullptr, m_dldi);
    LOG_TRACE("VkDebugUtilsMessenger created and set up.");
  }
  #endif
  
  void VulkanContext::chosePhysicalDevice() {
    vk::PhysicalDevice device = m_instance.enumeratePhysicalDevices(m_dldi).front();
    vk::PhysicalDeviceProperties properties = device.getProperties();
    LOG_INFO("Chose device " << properties.deviceName << " as VkPhysicalDevice.");
    m_device_physical = std::move(device);
  }

  void VulkanContext::choseDeviceQueues() {
    std::vector properties = m_device_physical.getQueueFamilyProperties(m_dldi);
    for (size_t i = 0; i < properties.size(); i++) {
      if(properties[i].queueCount > 0
        && properties[i].queueFlags & vk::QueueFlagBits::eGraphics 
        && properties[i].queueFlags & vk::QueueFlagBits::eTransfer) {
        m_graphics_queue_family_index = i;
        break;
      }
    }    
    for (size_t i = 0; i < properties.size(); i++) {
      if(properties[i].queueCount > 0
        && !(properties[i].queueFlags & vk::QueueFlagBits::eGraphics) 
        && properties[i].queueFlags & vk::QueueFlagBits::eTransfer
        && properties[i].queueFlags & vk::QueueFlagBits::eCompute) {
        m_has_compute_queue = true;
        m_compute_queue_family_index = i;
        LOG_INFO("Found dedicated compute queue family with index " << i << ".");
        break;
      }
    }
  }


  void VulkanContext::createLogicalDevice() {       
    //queues
    float queue_priority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queue_infos;
    
    vk::DeviceQueueCreateInfo graphics_queue_info(
      vk::DeviceQueueCreateFlags(),
      m_graphics_queue_family_index,
      1, &queue_priority);
    queue_infos.push_back(graphics_queue_info);
    
    if(m_has_compute_queue) {
      vk::DeviceQueueCreateInfo compute_queue_info(
        vk::DeviceQueueCreateFlags(),
        m_compute_queue_family_index,
        1, &queue_priority);
      queue_infos.push_back(compute_queue_info);
    }
    
    //extension
    std::vector<const char*> extensions; 
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);   
    #ifdef __APPLE__
    extensions.push_back("VK_KHR_portability_subset");
    #endif
    
    vk::DeviceCreateInfo create_info(vk::DeviceCreateFlags(), queue_infos.size(), queue_infos.data());
    create_info.setPEnabledExtensionNames(extensions);
    
    m_device = m_device_physical.createDevice(create_info);  
    LOG_TRACE("VkDevice has been created and queue family " << m_graphics_queue_family_index << " has been selected as graphics queue.");
  }

  void VulkanContext::getQueues() {
    m_device.getQueue(m_graphics_queue_family_index, 0, &m_graphics_queue, m_dldi);
    if(m_has_compute_queue) {
      m_device.getQueue(m_compute_queue_family_index, 0, &m_compute_queue, m_dldi);
    } else {
      m_compute_queue = m_graphics_queue;
      m_compute_queue_family_index = m_graphics_queue_family_index;
    }
  }

  void VulkanContext::createDescriptorPool() {
    std::vector<vk::DescriptorPoolSize> sizes = {
      {vk::DescriptorType::eStorageBuffer, 10},
      {vk::DescriptorType::eCombinedImageSampler, 5},
      {vk::DescriptorType::eStorageImage, 5}
    };          
    vk::DescriptorPoolCreateInfo create_info(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 5, sizes.size(), sizes.data());
    m_descriptor_pool = m_device.createDescriptorPool(create_info);
    LOG_TRACE("Created VkDescriptorPool.");
  }

  
  void VulkanContext::setupVulkanWindow() {
    m_window->createSurface(m_instance);
    m_window_data.Surface = m_window->getSurfaceKHR();
    if (!m_device_physical.getSurfaceSupportKHR(m_graphics_queue_family_index, m_window_data.Surface, m_dldi)) {
      LOG_ERROR("Window surface has no Vulkan support");
      exit(-1);
    }

    const VkFormat requestSurfaceImageFormat[] = { 
      VK_FORMAT_B8G8R8A8_UNORM, 
      VK_FORMAT_R8G8B8A8_UNORM, 
      VK_FORMAT_B8G8R8_UNORM, 
      VK_FORMAT_R8G8B8_UNORM 
    };

    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    m_window_data.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
      m_device_physical, 
      m_window_data.Surface, 
      requestSurfaceImageFormat, 
      (size_t) IM_ARRAYSIZE(requestSurfaceImageFormat), 
      requestSurfaceColorSpace);

    VkPresentModeKHR present_modes[] = { 
      VK_PRESENT_MODE_MAILBOX_KHR, 
      VK_PRESENT_MODE_IMMEDIATE_KHR, 
      VK_PRESENT_MODE_FIFO_KHR 
    };    
    m_window_data.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
      m_device_physical, 
      m_window_data.Surface, 
      &present_modes[0], 
      IM_ARRAYSIZE(present_modes));

    static_assert(c_min_image_count >= 2, "Min image count must at least be 2.");

    std::pair dimensions = m_window->getFrameBufferSize();
    ImGui_ImplVulkanH_CreateOrResizeWindow(
      m_instance, 
      m_device_physical, 
      m_device, 
      &m_window_data, 
      m_graphics_queue_family_index, 
      nullptr, 
      dimensions.first, 
      dimensions.second, 
      c_min_image_count);        
    LOG_TRACE("Vulkan window has been setup.");
  }  

  void VulkanContext::setupImGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    io.IniFilename = "config/imgui.ini";

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(m_window->getGlfwWindow(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_instance;
    init_info.PhysicalDevice = m_device_physical;
    init_info.Device = m_device;
    init_info.QueueFamily = m_graphics_queue_family_index;
    init_info.Queue = m_graphics_queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_descriptor_pool;
    init_info.RenderPass = m_window_data.RenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = c_min_image_count;
    init_info.ImageCount = m_window_data.ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = checkVkResult;
    ImGui_ImplVulkan_Init(&init_info);  
    LOG_TRACE("ImGUI has been setup.");

    
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    static auto s_window = m_window;
    static auto original_create_window = platform_io.Platform_CreateWindow;
    
    platform_io.Platform_CreateWindow = [](ImGuiViewport* viewport) {
      original_create_window(viewport);
      GLFWwindow* window = reinterpret_cast<GLFWwindow*>(viewport->PlatformHandle); 
      glfwSetWindowUserPointer(window, s_window);
      glfwSetDropCallback(window, s_window->glfwDropCallback);
    };
  }
  
  
  
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
    
    //begin render pass
    vk::RenderPassBeginInfo render_begin_info(
      m_window_data.RenderPass, 
      frame_data.Framebuffer, 
      vk::Rect2D(vk::Offset2D(0, 0), vk::Extent2D(m_window_data.Width, m_window_data.Height)),
      1, &c_background_color);
    cmd_buffer.beginRenderPass(render_begin_info, vk::SubpassContents::eInline, m_dldi);
    
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
  

  void VulkanContext::init(Window* window) {
    m_window = window;
    try {
      createVkInstance();
      #ifdef BUILD_DEBUG 
      createDebugMessenger();
      #endif
      chosePhysicalDevice();
      choseDeviceQueues();
      createLogicalDevice();
      getQueues();
      createDescriptorPool();
      setupVulkanWindow();
      setupImGUI();
    } catch(const vk::Error& err) {
      LOG_ERROR(err.what());
    } catch(...) {
      LOG_ERROR("Unknown error was thrown by Vulkan.");
    }
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

  void VulkanContext::destroy() {
    m_device.waitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplVulkanH_DestroyWindow(m_instance, m_device, &m_window_data, nullptr);
    m_device.destroyDescriptorPool(m_descriptor_pool);
    m_device.destroy();
    #ifdef BUILD_DEBUG
    m_instance.destroyDebugUtilsMessengerEXT(m_debug_messenger, nullptr, m_dldi);
    #endif
    m_instance.destroy();
  }
}
