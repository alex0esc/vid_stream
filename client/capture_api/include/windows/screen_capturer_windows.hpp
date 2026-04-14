#pragma once
#include "screen_capturer.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <memory>
#undef BUILD_WINDOWS
#include <wrl/client.h>
#define BUILD_WINDOWS
#pragma comment(lib, "dxgi.lib")
using Microsoft::WRL::ComPtr;


namespace vsa {


  class DisplayInfoWindows : public DisplayInfo {
  public:
    ComPtr<IDXGIAdapter1> m_adapter;
    ComPtr<IDXGIOutput> m_output;
  };


  class CapturerWindows : public Capturer {                
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGIOutputDuplication> m_duplication;
    ComPtr<IDXGIResource> m_resource;
    ComPtr<ID3D11Texture2D> m_staging_texture; // CPU readable copy

  public:
    std::vector<std::unique_ptr<DisplayInfo>> listDisplays() override;
    void init(std::unique_ptr<DisplayInfo>& display_info) override;
    bool captureFrame() override;
    void copyFrame(void* dst_memory) override;
    void destory() override;

  };
  
}
