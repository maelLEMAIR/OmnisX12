#ifndef COMMAND_QUEUE_H_INCLUDED
#define COMMAND_QUEUE_H_INCLUDED

#include "GpuDevice.h"
#include "../Resources/d3d12Utils.h"

class CommandQueue
{
public:
    CommandQueue() = default;
    ~CommandQueue();
    CommandQueue(const CommandQueue&) = delete;
    CommandQueue& operator=(const CommandQueue&) = delete;

    void Init(GpuDevice& gpuDevice);

    void ResetList(ID3D12PipelineState* pso = nullptr);
    void ResetListWithAllocator(ID3D12CommandAllocator* allocator, ID3D12PipelineState* pso = nullptr);
    void ExecuteList();

    
    UINT64 Signal();                             
    UINT64 GetCompletedValue() const;            
    bool   IsFenceComplete(UINT64 value) const;
    void   WaitForFenceValue(UINT64 value);    
    void   FlushCommandQueue();                  

    ID3D12CommandQueue*        GetQueue()       const { return m_commandQueue.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }

    UINT64 Report() { return 64; }
    
private:
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_currentFence = 0;
    HANDLE m_fenceEvent = nullptr;

    ComPtr<ID3D12CommandQueue>        m_commandQueue;
    ComPtr<ID3D12CommandAllocator>    m_directCmdListAlloc;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
};

#endif