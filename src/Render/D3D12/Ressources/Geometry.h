#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include <vector>
#include "d3d12Utils.h"

struct Vertex
{
    XMFLOAT3 pos;
    XMFLOAT4 color;
};

struct Geometry
{
    std::vector<Vertex> vertices;    
    std::vector<uint16_t> indices;
    const UINT vbByteSize;
    const UINT ibByteSize;
};

inline Geometry* BuildTriangle()
{
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;
        
    vertices = {
        //DOWN  Vertex
        Vertex{XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(Colors::Blue)},
        Vertex{XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT4(Colors::Red)},
        Vertex{XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT4(Colors::Green)}
            
    };
    indices = {
        0, 2, 1,
    };

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size() * sizeof(uint16_t);

    return new Geometry({vertices, indices, vbByteSize, ibByteSize});
}

struct SubmeshGeometry
{
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    INT BaseVertexLocation = 0;

    BoundingBox Bounds;
};

struct MeshGeometry
{
    std::string Name;

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

    std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

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