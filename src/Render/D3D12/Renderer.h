#pragma once
#include <windows.h>
#include "Core/MathUtils.hpp"
#include "Device/GpuDevice.h"
#include "Device/CommandQueue.h"
#include "Device/SwapChain.h"
#include "DirectX12/d3dx12.h"

class Renderer
{
public:
    Renderer() = default;
    ~Renderer();

    void Init(HWND hwnd, int width, int height);
    void OnResize(int width, int height);
    void Update(float dt);
    void Draw();

private:
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private:
    GpuDevice    m_device;
    CommandQueue m_commandQueue;
    SwapChain    m_swapChain;

    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12Resource>       m_depthStencilBuffer;
    DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    D3D12_VIEWPORT m_screenViewport = {};
    D3D12_RECT     m_scissorRect = {};
    int m_clientWidth  = 800;
    int m_clientHeight = 600;

    DirectX::XMFLOAT4X4 mWorld = MathUtils::Identity4x4();
    DirectX::XMFLOAT4X4 mView  = MathUtils::Identity4x4();
    DirectX::XMFLOAT4X4 mProj  = MathUtils::Identity4x4();

    // TODO (à rapatrier depuis WindowD3D12) : m_cbvHeap, m_objectCB, m_rootSignature,
    // mvsByteCode, mpsByteCode, mInputLayout, mBoxGeo, mPSOs
};