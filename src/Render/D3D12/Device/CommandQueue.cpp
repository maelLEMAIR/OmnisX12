#ifndef COMMAND_QUEUE_CPP_INCLUDED
#define COMMAND_QUEUE_CPP_INCLUDED

#include "CommandQueue.h"

CommandQueue::~CommandQueue()
{
    if (m_fenceEvent)
        CloseHandle(m_fenceEvent);
}

void CommandQueue::Init(GpuDevice& gpuDevice)
{
    ID3D12Device* device = gpuDevice.GetGpuDevice().Get();

    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
    m_fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_directCmdListAlloc)));

    ThrowIfFailed(device->CreateCommandList(
        0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_directCmdListAlloc.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

    ThrowIfFailed(m_commandList->Close());
}

void CommandQueue::ResetList(ID3D12PipelineState* pso)
{
    ThrowIfFailed(m_directCmdListAlloc->Reset());
    ThrowIfFailed(m_commandList->Reset(m_directCmdListAlloc.Get(), pso));
}

void CommandQueue::ResetListWithAllocator(ID3D12CommandAllocator* allocator, ID3D12PipelineState* pso)
{
    ThrowIfFailed(allocator->Reset());
    ThrowIfFailed(m_commandList->Reset(allocator, pso));
}

UINT64 CommandQueue::Signal()
{
    ++m_currentFence;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));
    return m_currentFence;
}

UINT64 CommandQueue::GetCompletedValue() const
{
    return m_fence->GetCompletedValue();
}

bool CommandQueue::IsFenceComplete(UINT64 value) const
{
    return m_fence->GetCompletedValue() >= value;
}

void CommandQueue::WaitForFenceValue(UINT64 value)
{
    if (IsFenceComplete(value))
        return;

    ThrowIfFailed(m_fence->SetEventOnCompletion(value, m_fenceEvent));
    WaitForSingleObject(m_fenceEvent, INFINITE);
}

void CommandQueue::FlushCommandQueue()
{
    WaitForFenceValue(Signal());
}

void CommandQueue::ExecuteList()
{
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* lists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(lists), lists);
}

#endif
