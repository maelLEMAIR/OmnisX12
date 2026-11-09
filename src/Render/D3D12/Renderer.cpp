#include "Renderer.h"

Renderer::~Renderer()
{
    
    if (m_commandQueue.GetQueue())
        m_commandQueue.FlushCommandQueue();
}

void Renderer::Init(HWND hwnd, int width, int height)
{
    m_clientWidth  = width;
    m_clientHeight = height;

    m_device.Init();                       
    m_commandQueue.Init(m_device);
    m_swapChain.Init(m_device, m_commandQueue, hwnd, width, height);

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type  = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(m_device.GetGpuDevice()->CreateDescriptorHeap(
        &dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

    OnResize(width, height);

    // m_commandQueue.ResetList();
    // BuildDescriptorHeaps();
    // BuildConstantBuffers();
    // BuildRootSignature();
    // m_commandQueue.ExecuteList();
    //
    // m_commandQueue.FlushCommandQueue();
}

void Renderer::OnResize(int width, int height)
{
    if (width == 0 || height == 0) return;
    m_clientWidth  = width;
    m_clientHeight = height;

    ID3D12Device* device = m_device.GetGpuDevice().Get();

    m_commandQueue.FlushCommandQueue();
    m_commandQueue.ResetList();

    m_depthStencilBuffer.Reset();
    m_swapChain.Resize(width, height);

    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width  = width;
    depthDesc.Height = height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags  = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear = {};
    optClear.Format = m_depthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(device->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &depthDesc,
        D3D12_RESOURCE_STATE_COMMON, &optClear,
        IID_PPV_ARGS(&m_depthStencilBuffer)));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = m_depthStencilFormat;
    device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_depthStencilBuffer.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    m_commandQueue.GetCommandList()->ResourceBarrier(1, &barrier);

    m_commandQueue.ExecuteList();
    m_commandQueue.FlushCommandQueue();

    m_screenViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
    m_scissorRect    = { 0, 0, width, height };

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathUtils::PI,
        (float)width / (float)height, 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void Renderer::Update(float dt)
{
}

void Renderer::Draw()
{
    ID3D12GraphicsCommandList* cmd = m_commandQueue.GetCommandList();

    m_commandQueue.ResetList(/* PSO "opaque" quand il sera rapatrié */ nullptr);

    cmd->RSSetViewports(1, &m_screenViewport);
    cmd->RSSetScissorRects(1, &m_scissorRect);

    CD3DX12_RESOURCE_BARRIER toRT = CD3DX12_RESOURCE_BARRIER::Transition(
        m_swapChain.CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmd->ResourceBarrier(1, &toRT);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_swapChain.CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE dsv = DepthStencilView();
    cmd->ClearRenderTargetView(rtv, Colors::LightSteelBlue, 0, nullptr);
    cmd->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    cmd->OMSetRenderTargets(1, &rtv, true, &dsv);

    // TODO : ici, SetDescriptorHeaps / RootSignature / buffers / topologie /
    //        DrawIndexedInstanced, copiés depuis WindowD3D12::Draw

    CD3DX12_RESOURCE_BARRIER toPresent = CD3DX12_RESOURCE_BARRIER::Transition(
        m_swapChain.CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmd->ResourceBarrier(1, &toPresent);

    m_commandQueue.ExecuteList();
    m_swapChain.Present();
    m_commandQueue.FlushCommandQueue();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::DepthStencilView() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}