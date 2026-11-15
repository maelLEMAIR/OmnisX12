#ifndef RENDERER_CPP_INCLUDED
#define RENDERER_CPP_INCLUDED

#include "Renderer.h"
#include "Core/define.h"

Renderer::Renderer() : m_mainCamera(XMFLOAT3(0.0f, 0.0f, 3.0f))
{
}

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

    m_commandQueue.ResetList();      
    
    BuildRootSignature();
    BuildFrameResources(); 
    BuildShadersAndInputLayout();
    CreateShader("Default", GetResPath() + L"Shaders/color.hlsl");

    m_commandQueue.ExecuteList();
    m_commandQueue.FlushCommandQueue();
}

void Renderer::BuildFrameResources()
{
    for(int i = 0; i < NumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(
        m_device.GetGpuDevice().Get(), 1, kMaxObjects));
    }
    mCurrFrameResource = mFrameResources[0].get();
}

void Renderer::BuildRootSignature()
{
    CD3DX12_ROOT_PARAMETER slots[2];
    slots[0].InitAsConstantBufferView(0);   
    slots[1].InitAsConstantBufferView(1);

    CD3DX12_ROOT_SIGNATURE_DESC rootDesc(2, slots, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> serialized, error;
    HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serialized.GetAddressOf(), error.GetAddressOf());
    if (error) OutputDebugStringA((char*)error->GetBufferPointer());
    ThrowIfFailed(hr);

    ThrowIfFailed(m_device.GetGpuDevice()->CreateRootSignature(
        0, serialized->GetBufferPointer(), serialized->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)));
}

void Renderer::BuildShadersAndInputLayout()
{
    m_vsByteCode = d3dUtil::CompileShader(GetResPath() + L"Shaders/color.hlsl", nullptr, "VS", "vs_5_0");
    m_psByteCode = d3dUtil::CompileShader(GetResPath() + L"Shaders/color.hlsl", nullptr, "PS", "ps_5_0");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
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

    mProj = m_mainCamera.GetProjMatrix();
}

void Renderer::Update(Chrono const& chrono)
{
    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % NumFrameResources;
    mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();
    
    if (mCurrFrameResource->Fence != 0)
        m_commandQueue.WaitForFenceValue(mCurrFrameResource->Fence);
    
    
    m_mainCamera.UpdateViewMatrix();
    
    mEyePos = m_mainCamera.GetPosition();
    mView = m_mainCamera.GetViewMatrix();
    
    UpdateMainPassCB(chrono);
}

void Renderer::UpdateMainPassCB(Chrono const& chrono)
{
    XMMATRIX view = mView;
    XMMATRIX proj = mProj;
    
    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    XMVECTOR viewDet = XMMatrixDeterminant(view);
    XMMATRIX invView = XMMatrixInverse(&viewDet, view);
    XMVECTOR projDet = XMMatrixDeterminant(proj);
    XMMATRIX invProj = XMMatrixInverse(&projDet, proj);
    XMVECTOR viewProjDet = XMMatrixDeterminant(viewProj);
    XMMATRIX invViewProj = XMMatrixInverse(&viewProjDet, viewProj);
    
    XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
    XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
    XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
    XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
    XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
    XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
    
    mMainPassCB.EyePosW = mEyePos;
    mMainPassCB.RenderTargetSize = XMFLOAT2((float)m_clientWidth, (float)m_clientHeight);
    mMainPassCB.InvRenderTargetSize = XMFLOAT2( 1.0f / (float)m_clientWidth, 1.0f / (float)m_clientHeight);
    mMainPassCB.NearZ = 1.0f;
    mMainPassCB.FarZ = 1'000.0f;
    mMainPassCB.TotalTime = (float)chrono.TotalTime();
    mMainPassCB.DeltaTime = chrono.DeltaTime();
    
    auto currPassCB = mCurrFrameResource->PassCB.get();
    currPassCB->CopyData(0, mMainPassCB);
}

void Renderer::Draw(MeshGeometry& geo, const XMFLOAT4X4& world, const XMFLOAT4& color, uint32 shaderId)
{
    if (m_drawCount >= kMaxObjects) return;
    if (shaderId >= m_shaders.size()) shaderId = 0;

    ObjectConstants constants;
    XMStoreFloat4x4(&constants.World, XMMatrixTranspose(XMLoadFloat4x4(&world)));
    constants.Color = color;
    mCurrFrameResource->ObjectCB->CopyData(m_drawCount, constants);

    ID3D12GraphicsCommandList* cmd = m_commandQueue.GetCommandList();

    if (shaderId != m_boundShader)
    {
        cmd->SetPipelineState(m_shaders[shaderId].pso.Get());
        m_boundShader = shaderId;
    }
    
    D3D12_VERTEX_BUFFER_VIEW vbv = geo.VertexBufferView();
    D3D12_INDEX_BUFFER_VIEW  ibv = geo.IndexBufferView();
    cmd->IASetVertexBuffers(0, 1, &vbv);
    cmd->IASetIndexBuffer(&ibv);
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const UINT stride = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    cmd->SetGraphicsRootConstantBufferView(0,
        mCurrFrameResource->ObjectCB->Resource()->GetGPUVirtualAddress() + (UINT64)m_drawCount * stride);

    cmd->DrawIndexedInstanced(geo.IndexCount, 1, 0, 0, 0);
    ++m_drawCount;
}

void Renderer::BeginDraw()
{
    m_drawCount = 0;

    for (size_t i = 0; i < m_pendingUploads.size(); )
    {
        if (m_commandQueue.IsFenceComplete(m_pendingUploads[i].fence))
        {
            m_pendingUploads[i].geo->DisposeUploaders();
            m_pendingUploads[i] = m_pendingUploads.back();
            m_pendingUploads.pop_back();
        }
        else
            ++i;
    }
    
    m_commandQueue.ResetListWithAllocator(mCurrFrameResource->CmdListAlloc.Get(), mPSOs["opaque"].Get());

    m_commandQueue.GetCommandList()->RSSetViewports(1, &m_screenViewport);
    m_commandQueue.GetCommandList()->RSSetScissorRects(1, &m_scissorRect);

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_swapChain.CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandQueue.GetCommandList()->ResourceBarrier(1, &barrier);

    m_commandQueue.GetCommandList()->ClearRenderTargetView(m_swapChain.CurrentBackBufferView(), m_clearColor, 0, nullptr);
    m_commandQueue.GetCommandList()->ClearDepthStencilView(DepthStencilView(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
    D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = m_swapChain.CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView();
    m_commandQueue.GetCommandList()->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);

    ID3D12GraphicsCommandList* cmd = m_commandQueue.GetCommandList();

    cmd->SetGraphicsRootSignature(m_rootSignature.Get());

    cmd->SetGraphicsRootConstantBufferView(1,
        mCurrFrameResource->PassCB->Resource()->GetGPUVirtualAddress());
}

void Renderer::EndDraw()
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_swapChain.CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandQueue.GetCommandList()->ResourceBarrier(1, &barrier);

    m_commandQueue.ExecuteList();
    m_swapChain.Present();

    mCurrFrameResource->Fence = m_commandQueue.Signal();
    
    for (MeshGeometry* g : m_newGeometries)
        m_pendingUploads.push_back({ g, mCurrFrameResource->Fence });
    m_newGeometries.clear();
    
    m_boundShader = -1;
}


uint32 Renderer::CreateShader(const std::string& name, const std::wstring& hlslPath)
{
    auto it = m_shaderIds.find(name);
    if (it != m_shaderIds.end())
        return it->second;

    Shader s;
    s.vs = d3dUtil::CompileShader(hlslPath, nullptr, "VS", "vs_5_0");
    s.ps = d3dUtil::CompileShader(hlslPath, nullptr, "PS", "ps_5_0");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout           = { mInputLayout.data(), (UINT)mInputLayout.size() };
    psoDesc.pRootSignature        = m_rootSignature.Get();
    psoDesc.VS                    = { s.vs->GetBufferPointer(), s.vs->GetBufferSize() };
    psoDesc.PS                    = { s.ps->GetBufferPointer(), s.ps->GetBufferSize() };
    psoDesc.RasterizerState       = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState            = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState     = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask            = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets      = 1;
    psoDesc.RTVFormats[0]         = m_swapChain.Format();
    psoDesc.SampleDesc.Count      = 1;
    psoDesc.DSVFormat             = m_depthStencilFormat;

    ThrowIfFailed(m_device.GetGpuDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&s.pso)));

    m_shaders.push_back(std::move(s));
    uint32 id = (uint32)m_shaders.size() - 1;
    m_shaderIds[name] = id;
    return id;
}

void Renderer::SetMainCamera(Camera* cam)
{
    if (cam == nullptr)
        return;

    m_mainCamera = *cam;
    m_mainCamera.UpdateViewMatrix();
    mProj = m_mainCamera.GetProjMatrix();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::DepthStencilView() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

MeshGeometry* Renderer::GetOrCreateGeometry(uint32 geoId, Geometry& geo)
{
    auto it = m_geometries.find(geoId);
    if (it != m_geometries.end())
        return it->second.get();

    if (geo.vertices.empty() || geo.indices.empty())
        return nullptr;

    const UINT vbSize = (UINT)(geo.vertices.size() * sizeof(Vertex));
    const UINT ibSize = (UINT)(geo.indices.size()  * sizeof(uint32_t));

    auto mesh = std::make_unique<MeshGeometry>();

    mesh->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
        m_device.GetGpuDevice().Get(), m_commandQueue.GetCommandList(),
        geo.vertices.data(), vbSize, mesh->VertexBufferUploader);
    mesh->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
        m_device.GetGpuDevice().Get(), m_commandQueue.GetCommandList(),
        geo.indices.data(), ibSize, mesh->IndexBufferUploader);

    mesh->VertexByteStride     = sizeof(Vertex);
    mesh->VertexBufferByteSize = vbSize;
    mesh->IndexFormat          = DXGI_FORMAT_R32_UINT;
    mesh->IndexBufferByteSize  = ibSize;
    mesh->IndexCount           = (UINT)geo.indices.size();

    MeshGeometry* raw = mesh.get();
    m_geometries[geoId] = std::move(mesh);
    m_newGeometries.push_back(raw);
    return raw;
}

#endif