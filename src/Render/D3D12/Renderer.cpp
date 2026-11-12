#ifndef RENDERER_CPP_INCLUDED
#define RENDERER_CPP_INCLUDED

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
    m_hwnd = hwnd;

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
    BuildShadersAndInputLayout();
    BuildBoxGeometry();
    BuildRenderObjects();           
    BuildFrameResources();                 
    BuildPSO();

    m_commandQueue.ExecuteList();
    m_commandQueue.FlushCommandQueue();

    m_boxGeo->DisposeUploaders();         
}

void Renderer::BuildBoxGeometry()
{
    // Unit cube centred at origin, vertex colors unused (overridden by CB)
    std::vector<Vertex> verts =
    {
        // Bottom face (y = -0.5)
        { {-0.5f, -0.5f, -0.5f}, {} }, { {-0.5f, -0.5f,  0.5f}, {} },
        { { 0.5f, -0.5f, -0.5f}, {} }, { { 0.5f, -0.5f,  0.5f}, {} },
        // Top face    (y = +0.5)
        { {-0.5f,  0.5f, -0.5f}, {} }, { {-0.5f,  0.5f,  0.5f}, {} },
        { { 0.5f,  0.5f, -0.5f}, {} }, { { 0.5f,  0.5f,  0.5f}, {} },
    };

    std::vector<uint16_t> idx =
    {
        0,2,1,  3,1,2,  // bottom
        4,5,6,  7,6,5,  // top
        0,1,4,  1,5,4,  // left
        2,6,3,  3,6,7,  // right
        0,4,6,  0,6,2,  // front
        1,7,5,  1,3,7,  // back
    };

    const UINT vbSize = (UINT)verts.size() * sizeof(Vertex);
    const UINT ibSize = (UINT)idx.size()   * sizeof(uint16_t);

    m_boxGeo = std::make_unique<MeshGeometry>();
    m_boxGeo->Name = "boxGeo";

    ThrowIfFailed(D3DCreateBlob(vbSize, &m_boxGeo->VertexBufferCPU));
    CopyMemory(m_boxGeo->VertexBufferCPU->GetBufferPointer(), verts.data(), vbSize);
    ThrowIfFailed(D3DCreateBlob(ibSize, &m_boxGeo->IndexBufferCPU));
    CopyMemory(m_boxGeo->IndexBufferCPU->GetBufferPointer(), idx.data(), ibSize);

    m_boxGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(
        m_device.GetGpuDevice().Get(), m_commandQueue.GetCommandList(), verts.data(), vbSize, m_boxGeo->VertexBufferUploader);
    m_boxGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(
        m_device.GetGpuDevice().Get(), m_commandQueue.GetCommandList(), idx.data(), ibSize, m_boxGeo->IndexBufferUploader);

    m_boxGeo->VertexByteStride     = sizeof(Vertex);
    m_boxGeo->VertexBufferByteSize = vbSize;
    m_boxGeo->IndexFormat          = DXGI_FORMAT_R16_UINT;
    m_boxGeo->IndexBufferByteSize  = ibSize;

    SubmeshGeometry sub;
    sub.IndexCount         = (UINT)idx.size();
    sub.StartIndexLocation = 0;
    sub.BaseVertexLocation = 0;
    m_boxGeo->DrawArgs["box"] = sub;
}

void Renderer::BuildFrameResources()
{
    for(int i = 0; i < NumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(
        m_device.GetGpuDevice().Get(), 1, (UINT)mAllRItems.size()));
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
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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

    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathUtils::PI,
        (float)width / (float)height, 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void Renderer::Update(Chrono const& chrono)
{
    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % NumFrameResources;
    mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

    mAllRItems[1].RotationY += chrono.DeltaTime();
    mAllRItems[1].NumFramesDirty = NumFrameResources;
    
    if (mCurrFrameResource->Fence != 0)
        m_commandQueue.WaitForFenceValue(mCurrFrameResource->Fence);
    
    mEyePos = XMFLOAT3(0.0f, 3.0f, -10.0f);
    XMVECTOR pos    = XMLoadFloat3(&mEyePos);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMStoreFloat4x4(&mView, XMMatrixLookAtLH(pos, target, up));

    UpdateObjectCBs(chrono);
    UpdateMainPassCB(chrono);
}

void Renderer::UpdateObjectCBs(Chrono const& chrono)
{
    auto currObjectCB = mCurrFrameResource->ObjectCB.get();
    for (auto& e : mAllRItems)
    {
        if (e.NumFramesDirty > 0)
        {
            XMMATRIX world = e.WorldMatrix();
            ObjectConstants objConstants;
            XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
            objConstants.Color = e.Color;                               
            currObjectCB->CopyData(e.ObjCBIndex, objConstants);
            
            e.NumFramesDirty--;
        }
    }
}

void Renderer::UpdateMainPassCB(Chrono const& chrono)
{
    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    
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

void Renderer::Draw()
{
    BeginDraw();
    DrawScene();
    EndDraw();
}

void Renderer::BeginDraw()
{
    m_commandQueue.ResetListWithAllocator(mCurrFrameResource->CmdListAlloc.Get(), mPSOs["opaque"].Get());

    m_commandQueue.GetCommandList()->RSSetViewports(1, &m_screenViewport);
    m_commandQueue.GetCommandList()->RSSetScissorRects(1, &m_scissorRect);

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_swapChain.CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandQueue.GetCommandList()->ResourceBarrier(1, &barrier);

    m_commandQueue.GetCommandList()->ClearRenderTargetView(m_swapChain.CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    m_commandQueue.GetCommandList()->ClearDepthStencilView(DepthStencilView(),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, nullptr);
    D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView = m_swapChain.CurrentBackBufferView();
    D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = DepthStencilView();
    m_commandQueue.GetCommandList()->OMSetRenderTargets(1, &currentBackBufferView, true, &depthStencilView);
}

void Renderer::DrawScene()
{
    ID3D12GraphicsCommandList* cmd = m_commandQueue.GetCommandList();

    cmd->SetGraphicsRootSignature(m_rootSignature.Get());

    cmd->SetGraphicsRootConstantBufferView(1,
        mCurrFrameResource->PassCB->Resource()->GetGPUVirtualAddress());

    D3D12_VERTEX_BUFFER_VIEW vbv = m_boxGeo->VertexBufferView();
    D3D12_INDEX_BUFFER_VIEW  ibv = m_boxGeo->IndexBufferView();
    cmd->IASetVertexBuffers(0, 1, &vbv);
    cmd->IASetIndexBuffer(&ibv);
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    const D3D12_GPU_VIRTUAL_ADDRESS objCBBase =
        mCurrFrameResource->ObjectCB->Resource()->GetGPUVirtualAddress();

    for (const RenderItem& item : mAllRItems)
    {
        cmd->SetGraphicsRootConstantBufferView(0,
            objCBBase + (UINT64)item.ObjCBIndex * objCBByteSize);

        cmd->DrawIndexedInstanced(item.IndexCount, 1,
            item.StartIndexLocation, item.BaseVertexLocation, 0);
    }
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
}

void Renderer::BuildRenderObjects()
{
    const SubmeshGeometry& box = m_boxGeo->DrawArgs["box"];

    mAllRItems.reserve(kNumBoxes);
    for (int i = 0; i < kNumBoxes; ++i)
    {
        const BoxDesc& d = kBoxDescs[i];

        RenderItem item;
        item.Position  = d.pos;
        item.Scale     = d.scale;
        item.RotationY = d.rotY;
        item.Color     = d.color;

        item.ObjCBIndex         = i;
        item.Geo                = m_boxGeo.get();
        item.IndexCount         = box.IndexCount;
        item.StartIndexLocation = box.StartIndexLocation;
        item.BaseVertexLocation = box.BaseVertexLocation;

        AddRenderItem(item);         
    }
}

void Renderer::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout    = { mInputLayout.data(), (UINT)mInputLayout.size() };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = { reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()), m_vsByteCode->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()), m_psByteCode->GetBufferSize() };
    psoDesc.RasterizerState       = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState            = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState     = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask            = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets      = 1;
    psoDesc.RTVFormats[0]         = m_swapChain.Format();
    psoDesc.SampleDesc.Count      = 1;
    psoDesc.SampleDesc.Quality    = 0;
    psoDesc.DSVFormat             = m_depthStencilFormat;

    ThrowIfFailed(m_device.GetGpuDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::DepthStencilView() const
{
    return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

#endif