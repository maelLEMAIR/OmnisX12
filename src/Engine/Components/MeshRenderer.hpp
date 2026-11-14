#ifndef MESH_RENDERER_HPP_INCLUDED
#define MESH_RENDERER_HPP_INCLUDED

#include "Core/define.h"
#include "Render/D3D12/Renderer.h"

struct MeshRenderer
{
    uint32   geoId = 0;
    uint32   materialId = 0;
    XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
};

#endif
