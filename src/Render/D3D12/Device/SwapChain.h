#ifndef SWAP_CHAIN_H_INCLUDED
#define SWAP_CHAIN_H_INCLUDED

#include "GpuDevice.h"
#include "CommandQueue.h"

class SwapChain
{
public:
    static constexpr UINT BufferCount = 2;

    void Init(GpuDevice& gpuDevice, CommandQueue& queue, HWND hwnd, UINT width, UINT height);

    void Resize(UINT width, UINT height);
    void Present();

    ID3D12Resource*                 CurrentBackBuffer()     const { return m_buffers[m_currBackBuffer].Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE     CurrentBackBufferView() const;
    DXGI_FORMAT                     Format()                const { return m_backBufferFormat; }
    bool                            MsaaState()             const { return m_4xMsaaState; }
    UINT                            GetMsaaQuality()        const { return m_4xMsaaQuality; }

private:
    void CreateBuffersAndViews();

    ComPtr<ID3D12Device>         m_device;
    ComPtr<IDXGISwapChain>       m_swapChain;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12Resource>       m_buffers[BufferCount];

    bool m_4xMsaaState = false;
    UINT m_4xMsaaQuality = 0;
    
    UINT m_rtvDescriptorSize = 0;
    UINT m_currBackBuffer = 0;
    DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
};

#endif