#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include <vector>
#include "../Resources/d3d12Utils.h"

struct Vertex
{
    XMFLOAT3 pos;
    XMFLOAT2 uv;
    XMFLOAT3 normal;
    XMFLOAT3 tangent;
    XMFLOAT4 color;
};

struct Geometry
{
    std::vector<Vertex> vertices;    
    std::vector<uint32_t> indices;
    const UINT vbByteSize = 0;
    const UINT ibByteSize = 0;
    
    Geometry() { };
};

struct MeshGeometry
{
    BoundingBox Bounds;
    
    UINT IndexCount = 0;
    
    ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
    ComPtr<ID3DBlob> IndexBufferCPU  = nullptr;

    ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
    ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

    ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
    ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

    UINT VertexByteStride = 0;
    UINT VertexBufferByteSize = 0;
    DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
    UINT IndexBufferByteSize = 0;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
    {
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
        vbv.StrideInBytes = VertexByteStride;
        vbv.SizeInBytes = VertexBufferByteSize;

        return vbv;
    }

    D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
    {
        D3D12_INDEX_BUFFER_VIEW ibv;
        ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
        ibv.Format = IndexFormat;
        ibv.SizeInBytes = IndexBufferByteSize;

        return ibv;
    }

    void DisposeUploaders()
    {
        VertexBufferUploader = nullptr;
        IndexBufferUploader = nullptr;
    }
};
#endif