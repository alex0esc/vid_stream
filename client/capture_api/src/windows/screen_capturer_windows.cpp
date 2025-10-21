#include "windows/screen_capturer_windows.hpp"
#include "logger.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <dxgiformat.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>
#include <winerror.h>


namespace vsa {

  void CapturerWindows::init() {
    HRESULT result = D3D11CreateDevice(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
      nullptr,
      0,
      D3D11_SDK_VERSION,
      &m_device,
      nullptr,
      &m_context);
    if(FAILED(result)) {
      LOG_ERROR("Failed to create D3D11 device.");
      std::abort();
    }
    
    m_device.As(&m_dxgi_device);

    m_dxgi_device->GetAdapter(&m_adapter);

    m_adapter->EnumOutputs(0, &m_output);
    m_output.As(&m_output1);
    
    result = m_output1->DuplicateOutput(m_device.Get(), &m_duplication);
    if(FAILED(result)) {
      LOG_ERROR("Duplication failed with code: " << std::hex << result);
      std::abort();
    }
  }

  
  bool CapturerWindows::captureFrame() {
    m_duplication->ReleaseFrame();
    DXGI_OUTDUPL_FRAME_INFO frame_info = {};
    HRESULT result = m_duplication->AcquireNextFrame(100, &frame_info, &m_desktop_resource);
    if(result == DXGI_ERROR_WAIT_TIMEOUT) {
      LOG_ERROR("Not enough time to aquire next frame."); 
      return false;
    } else if(FAILED(result)) {
      LOG_ERROR("Could not aquire next frame."); 
      return false;
    }
    m_desktop_resource.As(&m_texture);
    D3D11_TEXTURE2D_DESC desc = {};
    m_texture->GetDesc(&desc);
    m_texture_width = desc.Width;
    m_texture_height = desc.Height;
    return true;
  }


  uint8_t* CapturerWindows::getFrame() {
    return nullptr;
  }
  /*
  bool CapturerWindows::getVulkanFrame(uif::TextureData& texture) {
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = m_texture_width;
    desc.Height = m_texture_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    HRESULT result = m_device->CreateTexture2D(&desc, nullptr, &m_shared_texture);
    if(FAILED(result)) {
      LOG_ERROR("Failed to create the shared texture for vulkan.");
      return false;
    }

    m_shared_texture.As(&m_dxgi_resource);

    HANDLE shared_handle = nullptr;
    m_dxgi_resource->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ, nullptr, &shared_handle);

    vk::ExternalMemoryImageCreateInfo external_info(vk::ExternalMemoryHandleTypeFlagBits::eD3D11Texture);

    vk::ImageCreateInfo image_info;
    image_info.imageType = vk::ImageType::e2D;
    image_info.pNext = &external_info;
    image_info.format = vk::Format::eR8G8B8A8Unorm;
    image_info.extent = vk::Extent3D(m_texture_width, m_texture_height, 1);
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = vk::SampleCountFlagBits::e1;
    image_info.tiling = vk::ImageTiling::eOptimal;
    image_info.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
     
    return true;
  }
  */

  void CapturerWindows::destory() {
    m_texture_width = 0;
    m_texture_height = 0;
    m_texture.Reset();
    m_desktop_resource.Reset();
    m_duplication.Reset();
    m_output1.Reset();
    m_output.Reset();
    m_adapter.Reset();
    m_dxgi_device.Reset();
    m_device.Reset();
  }
  
}
