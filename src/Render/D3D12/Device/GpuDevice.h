#ifndef GPU_DEVICE_H_INCLUDED
#define GPU_DEVICE_H_INCLUDED

#include "../Ressources/d3d12Utils.h"

class GpuDevice
{
public:
    void Init();
    
    ComPtr<ID3D12Device>& GetGpuDevice() { return m_device; }
    ComPtr<IDXGIFactory4>& GetFactory()  { return m_factory; }
    
    UINT RtvDescriptorSize()       const { return m_rtvDescriptorSize; }
    UINT DsvDescriptorSize()       const { return m_dsvDescriptorSize; }
    UINT CbvSrvUavDescriptorSize() const { return m_cbvSrvUavDescriptorSize; }

protected:
    
private:
    ComPtr<ID3D12Device>  m_device;
    ComPtr<IDXGIFactory4> m_factory;
    
    UINT m_rtvDescriptorSize = 0;
    UINT m_dsvDescriptorSize = 0;
    UINT m_cbvSrvUavDescriptorSize = 0;
};


#endif
