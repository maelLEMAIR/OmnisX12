#ifndef FRAME_RESOURCE_H_INCLUDED
#define FRAME_RESOURCE_H_INCLUDED

#include "Core/MathUtils.hpp"
#include "../Resources/d3d12Utils.h"
#include "../Resources/UploadBuffer.h"

struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = MathUtils::Identity4x4();
    DirectX::XMFLOAT4   Color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

struct PassConstants
{
    DirectX::XMFLOAT4X4 View = MathUtils::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = MathUtils::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathUtils::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = MathUtils::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathUtils::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = MathUtils::Identity4x4();
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;
};

struct MaterialConstants
{
    DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    float Roughness = 0.25f;
    
    DirectX::XMFLOAT4X4 MatTransform = MathUtils::Identity4x4();
};

struct FrameResource
{
public:
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();
    
    ComPtr<ID3D12CommandAllocator> CmdListAlloc;
    
    std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
    std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
    std::unique_ptr<UploadBuffer<MaterialConstants>> MaterialCB = nullptr;
    
    UINT64 Fence = 0;
};


#endif