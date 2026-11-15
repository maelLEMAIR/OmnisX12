#ifndef MATERIAL_H_INCLUDED
#define MATERIAL_H_INCLUDED

#include "../Resources/d3d12Utils.h"

#include "Core/MathUtils.hpp"

struct Material
{
    std::string Name;
    
    int MatCBIndex = -1;
    
    int NumFramesDirty = NumFrameResources;
    DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
    float Roughness = 0.25f;
    
    DirectX::XMFLOAT4X4 MatTransform = MathUtils::Identity4x4();
};

#endif