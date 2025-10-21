#pragma once
#include "screen_capturer.hpp"
#include "d3d11.h"
#include "dxgi1_2.h"
#include "wrl/client.h"

using Microsoft::WRL::ComPtr;


namespace vsa {

  class CapturerWindows : public Capturer {
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;

    ComPtr<IDXGIDevice> m_dxgi_device;
    ComPtr<IDXGIAdapter> m_adapter;
    ComPtr<IDXGIOutput> m_output;
    ComPtr<IDXGIOutput1> m_output1;
    
    ComPtr<IDXGIOutputDuplication> m_duplication;
    ComPtr<IDXGIResource> m_desktop_resource;

    uint32_t m_texture_width = 0;
    uint32_t m_texture_height = 0;
    ComPtr<ID3D11Texture2D> m_texture;
    
    ComPtr<ID3D11Texture2D> m_shared_texture;
    ComPtr<IDXGIResource1> m_dxgi_resource;
    
     
  public:
    void init() override;
    bool captureFrame() override;
    uint8_t* getFrame() override;
    void destory() override;
  };
}
