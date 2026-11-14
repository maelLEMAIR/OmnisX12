#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <windows.h>

#include "Camera.h"
#include "Core/MathUtils.hpp"
#include "Core/Chrono.h"

#include "Device/GpuDevice.h"
#include "Device/CommandQueue.h"
#include "Device/FrameResource.h"
#include "Device/SwapChain.h"

#include "DirectX12/d3dx12.h"
#include "Items/RenderItem.h"

struct BoxDesc { XMFLOAT3 pos; XMFLOAT3 scale; float rotY; XMFLOAT4 color; };

static const BoxDesc kBoxDescs[] =
{
    { {-3.5f, 0.f, 0.f},  {1.f, 1.f,  1.f},  0.f,         {1.f,  0.2f, 0.2f, 1.f} },
    { { 0.f,  0.f, 0.f},  {1.f, 1.5f, 1.f},  XM_PIDIV4,   {0.2f, 1.f,  0.2f, 1.f} },
    { { 3.5f, 0.f, 0.f},  {1.f, 1.f,  1.f},  0.f,         {0.2f, 0.4f, 1.f,  1.f} },
};

static const int kNumBoxes = (int)std::size(kBoxDescs);

class Renderer
{
public:
    Renderer() = default;
    ~Renderer();

    void Init(HWND hwnd, int width, int height);
    void OnResize(int width, int height);
    
    void Update(Chrono const& chrono);
    
    void BeginDraw();
    MeshGeometry* GetOrCreateGeometry(uint32_t geoId, Geometry& geo);
    void Draw(MeshGeometry& geo, const XMFLOAT4X4& world, const XMFLOAT4& color);
    void EndDraw();
    
    GpuDevice& GetGpuDevice() { return m_device; }
    void SetClearColor(XMFLOAT3 clearColor) { m_clearColor = { { { clearColor.x, clearColor.y, clearColor.z, 1.0f } } }; }
    void SetMainCamera(Camera* cam) { m_mainCamera = *cam; }

private:
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

private:
    void UpdateObjectCBs(Chrono const& chrono);
    void UpdateMainPassCB(Chrono const& chrono);
    
    void BuildRootSignature();
    void BuildFrameResources();
    void BuildShadersAndInputLayout();
    void BuildPSO();

    XMVECTORF32 m_clearColor = Colors::BlueViolet;
    
    ComPtr<ID3D12RootSignature>  m_rootSignature = nullptr;
    
    ComPtr<ID3DBlob> m_vsByteCode = nullptr;
    ComPtr<ID3DBlob> m_psByteCode = nullptr;

    GpuDevice    m_device;
    CommandQueue m_commandQueue;
    SwapChain    m_swapChain;
    
    static const int NumFrameResources = 3;
    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;
    
    static constexpr UINT kMaxObjects = 4096; 
    UINT m_drawCount = 0;

    std::unordered_map<uint32_t, std::unique_ptr<MeshGeometry>> m_geometries;

    struct PendingUpload { MeshGeometry* geo; UINT64 fence; };
    std::vector<MeshGeometry*> m_newGeometries;
    std::vector<PendingUpload> m_pendingUploads;
    
    
    PassConstants mMainPassCB;
    XMFLOAT3 mEyePos = XMFLOAT3(0.0f, 0.0f, 0.0f);
    
    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
    
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12Resource>       m_depthStencilBuffer;
    DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    
    D3D12_VIEWPORT m_screenViewport = {};
    D3D12_RECT     m_scissorRect = {};
    int m_clientWidth  = 800;
    int m_clientHeight = 600;

    Camera m_mainCamera;
    
    DirectX::XMFLOAT4X4 mView  = MathUtils::Identity4x4();
    DirectX::XMFLOAT4X4 mProj  = MathUtils::Identity4x4();
};

#endif
