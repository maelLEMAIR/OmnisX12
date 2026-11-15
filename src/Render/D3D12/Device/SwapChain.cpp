#ifndef SWAP_CHAIN_CPP_INCLUDED
#define SWAP_CHAIN_CPP_INCLUDED

#include "SwapChain.h"

#include "D3D12/DirectX12/d3dx12.h"


void SwapChain::Init(GpuDevice& gpuDevice, CommandQueue& queue, HWND hwnd, UINT width, UINT height)
{
    m_device = gpuDevice.GetGpuDevice();
    m_rtvDescriptorSize = gpuDevice.RtvDescriptorSize();
 
    m_swapChain.Reset();
 
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferDesc.Width  = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format           = m_backBufferFormat;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling          = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count   = m_4xMsaaState ? 4 : 1;
    sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
    sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount  = BufferCount;
    sd.OutputWindow = hwnd;
    sd.Windowed     = true;
    sd.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags        = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
 
    ThrowIfFailed(gpuDevice.GetFactory()->CreateSwapChain(
        queue.GetQueue(), &sd, m_swapChain.GetAddressOf()));
 
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = BufferCount;
    rtvHeapDesc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
 
    CreateBuffersAndViews();
}

void SwapChain::Resize(UINT width, UINT height)
{
    for (UINT i = 0; i < BufferCount; ++i)
        m_buffers[i].Reset();
 
    ThrowIfFailed(m_swapChain->ResizeBuffers(
        BufferCount, width, height, m_backBufferFormat,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
 
    m_currBackBuffer = 0;
    CreateBuffersAndViews();
}

void SwapChain::CreateBuffersAndViews()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < BufferCount; ++i)
    {
        ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i])));
        m_device->CreateRenderTargetView(m_buffers[i].Get(), nullptr, handle);
        handle.Offset(1, m_rtvDescriptorSize);
    }
}

void SwapChain::Present()
{
    ThrowIfFailed(m_swapChain->Present(0, 0));
    m_currBackBuffer = (m_currBackBuffer + 1) % BufferCount;
}
 
D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::CurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_currBackBuffer, m_rtvDescriptorSize);
}

#endif
