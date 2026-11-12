#ifndef RENDER_ITEM_H_INCLUDED
#define RENDER_ITEM_H_INCLUDED

#include "Core/MathUtils.hpp"
#include "../Ressources/d3d12Utils.h"
#include "../DirectX12/d3dx12.h"
#include "../Ressources/Geometry.h"

struct RenderItem
{
    XMFLOAT3 Position  = { 0.f, 0.f, 0.f };
    XMFLOAT3 Scale     = { 1.f, 1.f, 1.f };
    float    RotationY = 0.f;
    XMFLOAT4 Color     = { 1.f, 1.f, 1.f, 1.f };

    XMMATRIX WorldMatrix() const
    {
        XMMATRIX S = XMMatrixScaling(Scale.x, Scale.y, Scale.z);
        XMMATRIX R = XMMatrixRotationY(RotationY);
        XMMATRIX T = XMMatrixTranslation(Position.x, Position.y, Position.z);
        return S * R * T;
    }

    int NumFramesDirty = 0;

    UINT ObjCBIndex = 0;

    MeshGeometry* Geo = nullptr;
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int  BaseVertexLocation = 0;
};

#endif