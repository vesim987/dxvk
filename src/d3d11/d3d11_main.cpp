#include <array>

#include <dxgi_adapter.h>
#include <dxgi_device.h>

#include "d3d11_device.h"

extern "C" {
  using namespace dxvk;
  
  DLLEXPORT HRESULT __stdcall D3D11CreateDevice(
          IDXGIAdapter        *pAdapter,
          D3D_DRIVER_TYPE     DriverType,
          HMODULE             Software,
          UINT                Flags,
    const D3D_FEATURE_LEVEL   *pFeatureLevels,
          UINT                FeatureLevels,
          UINT                SDKVersion,
          ID3D11Device        **ppDevice,
          D3D_FEATURE_LEVEL   *pFeatureLevel,
          ID3D11DeviceContext **ppImmediateContext) {
    TRACE(pAdapter, DriverType, Software,
          Flags, pFeatureLevels, FeatureLevels,
          SDKVersion, ppDevice, pFeatureLevel,
          ppImmediateContext);
    
    Com<IDXGIAdapter> dxgiAdapter = pAdapter;
    Com<IDXVKAdapter> dxvkAdapter = nullptr;
    
    if (dxgiAdapter == nullptr) {
      // We'll treat everything as hardware, even if the
      // Vulkan device is actually a software device.
      if (DriverType != D3D_DRIVER_TYPE_HARDWARE) {
        Logger::err("D3D11CreateDevice: Unsupported driver type");
        return DXGI_ERROR_UNSUPPORTED;
      }
      
      // We'll use the first adapter returned by a DXGI factory
      Com<IDXGIFactory> factory = nullptr;
      
      if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory)))) {
        Logger::err("D3D11CreateDevice: Failed to create a DXGI factory");
        return E_FAIL;
      }
      
      if (FAILED(factory->EnumAdapters(0, &dxgiAdapter))) {
        Logger::err("D3D11CreateDevice: No default adapter available");
        return E_FAIL;
      }
      
    } else {
      // In theory we could ignore these, but the Microsoft docs explicitly
      // state that we need to return E_INVALIDARG in case the arguments are
      // invalid. Both the driver type and software parameter can only be
      // set if the adapter itself is unspecified.
      // See: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476082(v=vs.85).aspx
      if (DriverType != D3D_DRIVER_TYPE_UNKNOWN || Software != nullptr)
        return E_INVALIDARG;
    }
    
    // The adapter must obviously be a DXVK-compatible adapter so
    // that we can create a DXVK-compatible DXGI device from it.
    if (FAILED(dxgiAdapter->QueryInterface(__uuidof(IDXVKAdapter),
        reinterpret_cast<void**>(&dxvkAdapter)))) {
      Logger::err("D3D11CreateDevice: Adapter is not a DXVK adapter");
      return E_FAIL;
    }
    
    // Feature levels to probe if the
    // application does not specify any.
    std::array<D3D_FEATURE_LEVEL, 6> defaultFeatureLevels = {
      D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3,
      D3D_FEATURE_LEVEL_9_2,  D3D_FEATURE_LEVEL_9_1,
    };
    
    if (pFeatureLevels == nullptr || FeatureLevels == 0) {
      pFeatureLevels = defaultFeatureLevels.data();
      FeatureLevels  = defaultFeatureLevels.size();
    }
    
    // Find the highest feature level supported by the device.
    // This works because the feature level array is ordered.
    UINT flId;
    for (flId = 0 ; flId < FeatureLevels; flId++) {
      if (D3D11Device::CheckFeatureLevelSupport(pFeatureLevels[flId]))
        break;
    }
    
    if (flId == FeatureLevels) {
      Logger::err("D3D11CreateDevice: Requested feature level not supported");
      return DXGI_ERROR_UNSUPPORTED;
    }
    
    // Try to create the device with the given parameters.
    const D3D_FEATURE_LEVEL fl = pFeatureLevels[flId];
    
    try {
      
      // Write back the actual feature level
      // if the application requested it.
      if (pFeatureLevel != nullptr)
        *pFeatureLevel = fl;
      
      // The documentation is unclear about what exactly should be done if
      // the application passes NULL to ppDevice, but a non-NULL pointer to
      // ppImmediateContext. In our implementation, the immediate context
      // does not hold a strong reference to the device that owns it, so
      // if we cannot write back the device, it would be destroyed.
      if (ppDevice != nullptr) {
        Com<IDXVKDevice> dxvkDevice = nullptr;
        
        if (FAILED(DXGICreateDXVKDevice(dxvkAdapter.ptr(), &dxvkDevice))) {
          Logger::err("D3D11CreateDevice: Failed to create DXGI device");
          return E_FAIL;
        }
        
        Com<D3D11Device> d3d11Device = new D3D11Device(
          dxvkDevice.ptr(), fl, Flags);
        
        *ppDevice = d3d11Device.ref();
        if (ppImmediateContext != nullptr)
          d3d11Device->GetImmediateContext(ppImmediateContext);
        return S_OK;
      } else {
        Logger::warn("D3D11CreateDevice: ppDevice is null");
        return S_OK;
      }
      
    } catch (const DxvkError& e) {
      Logger::err("D3D11CreateDevice: Failed to create D3D11 device");
      return E_FAIL;
    }
  }
  
  
  DLLEXPORT HRESULT __stdcall D3D11CreateDeviceAndSwapChain(
          IDXGIAdapter         *pAdapter,
          D3D_DRIVER_TYPE      DriverType,
          HMODULE              Software,
          UINT                 Flags,
    const D3D_FEATURE_LEVEL    *pFeatureLevels,
          UINT                 FeatureLevels,
          UINT                 SDKVersion,
    const DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
          IDXGISwapChain       **ppSwapChain,
          ID3D11Device         **ppDevice,
          D3D_FEATURE_LEVEL    *pFeatureLevel,
          ID3D11DeviceContext  **ppImmediateContext) {
    TRACE(pAdapter, DriverType, Software,
          Flags, pFeatureLevels, FeatureLevels,
          SDKVersion, pSwapChainDesc, ppSwapChain,
          ppDevice, pFeatureLevel, ppImmediateContext);
    
    // Try to create a device first.
    HRESULT status = D3D11CreateDevice(pAdapter, DriverType,
      Software, Flags, pFeatureLevels, FeatureLevels,
      SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
    
    if (FAILED(status))
      return status;
    
    // Again, the documentation does not exactly tell us what we
    // need to do in case one of the arguments is a null pointer.
    if (ppDevice != nullptr && ppSwapChain != nullptr) {
      if (pSwapChainDesc == nullptr)
        return E_INVALIDARG;
      
      Com<ID3D11Device> d3d11Device = *ppDevice;
      Com<IDXGIDevice>  dxgiDevice  = nullptr;
      Com<IDXGIAdapter> dxgiAdapter = nullptr;
      Com<IDXGIFactory> dxgiFactory = nullptr;
      
      if (FAILED(d3d11Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice)))) {
        Logger::err("D3D11CreateDeviceAndSwapChain: Failed to query DXGI device");
        return E_FAIL;
      }
      
      if (FAILED(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter)))) {
        Logger::err("D3D11CreateDeviceAndSwapChain: Failed to query DXGI adapter");
        return E_FAIL;
      }
      
      if (FAILED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgiFactory)))) {
        Logger::err("D3D11CreateDeviceAndSwapChain: Failed to query DXGI factory");
        return E_FAIL;
      }
      
      DXGI_SWAP_CHAIN_DESC desc = *pSwapChainDesc;
      if (FAILED(dxgiFactory->CreateSwapChain(d3d11Device.ptr(), &desc, ppSwapChain))) {
        Logger::err("D3D11CreateDeviceAndSwapChain: Failed to create swap chain");
        return E_FAIL;
      }
      
      return S_OK;
    } else {
      Logger::warn("D3D11CreateDeviceAndSwapChain: Not creating a swap chain");
      return S_OK;
    }
  }
  
}