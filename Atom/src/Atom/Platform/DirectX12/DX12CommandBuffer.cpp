#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12CommandBuffer.h"
#include "DX12Device.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12CommandBuffer::DX12CommandBuffer()
    {
        auto d3dDevice = Renderer::GetDevice().As<DX12Device>()->GetD3DDevice();
        u32 framesInFlight = Renderer::GetFramesInFlight();

        // Create command allocators for each frame
        m_Allocators.resize(framesInFlight);
        for (u32 i = 0; i < framesInFlight; i++)
        {
            DXCall(d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_Allocators[i])));
        }

        // Create command list
        DXCall(d3dDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList)));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12CommandBuffer::~DX12CommandBuffer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::Begin()
    {
        u32 frameIndex = Renderer::GetCurrentFrameIndex();
        DXCall(m_Allocators[frameIndex]->Reset());
        DXCall(m_CommandList->Reset(m_Allocators[frameIndex].Get(), NULL));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandBuffer::End()
    {
        DXCall(m_CommandList->Close());
    }
}

#endif // ATOM_PLATFORM_WINDOWS
