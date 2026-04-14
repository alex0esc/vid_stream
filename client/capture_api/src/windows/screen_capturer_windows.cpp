#include "windows/screen_capturer_windows.hpp"
#include "screen_capturer.hpp"
#include "logger.hpp"
#include <d3d11.h>
#include <dxgiformat.h>


namespace vsa {

  std::vector<std::unique_ptr<DisplayInfo>> CapturerWindows::listDisplays() {
    ComPtr<IDXGIFactory1> factory = nullptr;
    CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);
  
    ComPtr<IDXGIAdapter1> adapter = nullptr;
    std::vector<std::unique_ptr<DisplayInfo>> display_infos;    
    for (uint32_t i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
      ComPtr<IDXGIOutput> output = nullptr;
      for (UINT j = 0; adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND; j++) {
  
        DXGI_OUTPUT_DESC outputDesc;
        output->GetDesc(&outputDesc);
        DisplayInfoWindows* info = new DisplayInfoWindows();
        auto wstr = std::wstring(outputDesc.DeviceName);
        info->m_name = std::string(wstr.begin(), wstr.end()).substr(4);
        info->m_offset_x = outputDesc.DesktopCoordinates.left;
        info->m_offset_y = outputDesc.DesktopCoordinates.top;
        info->m_width = outputDesc.DesktopCoordinates.right - info->m_offset_x;
        info->m_height = outputDesc.DesktopCoordinates.bottom - info->m_offset_y;
        info->m_adapter = adapter; 
        info->m_output = output;
        display_infos.push_back(std::unique_ptr<DisplayInfo>(info));       
        LOG_TRACE("Found display " << info->m_name << ": " << info->m_width << "x" << info->m_height << ".");
      }
    }
    return display_infos;
  }
  
  void CapturerWindows::init(std::unique_ptr<DisplayInfo>& display_info) {
    DisplayInfoWindows* info_windows = static_cast<DisplayInfoWindows*>(display_info.get());
    D3D11CreateDevice(
      info_windows->m_adapter.Get(),
      D3D_DRIVER_TYPE_UNKNOWN,
      nullptr, 0,
      nullptr, 0,
      D3D11_SDK_VERSION,
      &m_device,
      nullptr,
      &m_context);
    LOG_TRACE("Created dxgi D3D11Device.");
    
    ComPtr<IDXGIOutput1> output1;
    info_windows->m_output.As(&output1);
    output1->DuplicateOutput(m_device.Get(), &m_duplication);      

    //create staging buffer
    //TODO only init if necessary
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = display_info->m_width;
    desc.Height = display_info->m_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.SampleDesc.Count = 1;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    m_device->CreateTexture2D(&desc, nullptr, &m_staging_texture);
  }
  
  bool CapturerWindows::captureFrame() {
    m_resource.Reset();
    m_duplication->ReleaseFrame();
    DXGI_OUTDUPL_FRAME_INFO frame_info;
    HRESULT result = m_duplication->AcquireNextFrame(500, &frame_info, &m_resource);
    if(!SUCCEEDED(result)) {
      LOG_ERROR("DXGI error while capturing a frame: " << result);
      return false;
    }
    return true;
  }
  
  void CapturerWindows::copyFrame(void* dst_memory) {
    ComPtr<ID3D11Texture2D> gpu_texture; 
    m_resource.As(&gpu_texture);

    m_context->CopyResource(m_staging_texture.Get(), gpu_texture.Get());
    D3D11_MAPPED_SUBRESOURCE mapped;
    m_context->Map(m_staging_texture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    memcpy(dst_memory, mapped.pData, mapped.DepthPitch);
    m_context->Unmap(m_staging_texture.Get(), 0);
    gpu_texture.Reset();
  }
  
  void CapturerWindows::destory() {
    m_staging_texture.Reset();
    m_resource.Reset();
    m_duplication.Reset();
    m_device.Reset();
    m_context.Reset();
  }  

}
