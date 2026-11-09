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

void CommandQueue::ExecuteList()
{
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* lists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(lists), lists);
}

void CommandQueue::FlushCommandQueue()
{
    m_currentFence++;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));

    if (m_fence->GetCompletedValue() < m_currentFence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

#endif
