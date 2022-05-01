#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"

#include "Buffer.h"
#include "Device.h"
#include "CommandBuffer.h"
#include "CommandQueue.h"
#include "ResourceStateTracker.h"
#include "Renderer.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    VertexBuffer::VertexBuffer(const VertexBufferDescription& description, const char* debugName)
        : m_Description(description)
    {
        auto d3dDevice = Renderer::GetDevice()->GetD3DDevice();

        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(GetSize());

        DXCall(d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_D3DResource)));

        // If we supply any data, create an upload resource
        if (description.Data)
        {
            ComPtr<ID3D12Resource> uploadBuffer = nullptr;

            DXCall(d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

            D3D12_SUBRESOURCE_DATA vertexData = {};
            vertexData.pData = description.Data;
            vertexData.RowPitch = GetSize();
            vertexData.SlicePitch = vertexData.RowPitch;

            Ref<CommandBuffer> commandBuffer = CreateRef<CommandBuffer>("CopyCommandBuffer");
            commandBuffer->Begin();
            auto commandList = commandBuffer->GetCommandList();

            UpdateSubresources<1>(commandList.Get(), m_D3DResource.Get(), uploadBuffer.Get(), 0, 0, 1, &vertexData);

            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_D3DResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

            commandBuffer->End();
            u64 fenceValue = Renderer::GetDevice()->GetCommandQueue(CommandQueueType::Graphics)->ExecuteCommandList(commandBuffer.get());
            Renderer::GetDevice()->GetCommandQueue(CommandQueueType::Graphics)->WaitForFenceValue(fenceValue);
        }

        // Create buffer view
        m_View.BufferLocation = m_D3DResource->GetGPUVirtualAddress();
        m_View.SizeInBytes = GetSize();
        m_View.StrideInBytes = m_Description.VertexStride;

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        ResourceStateTracker::AddGlobalResourceState(m_D3DResource.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    VertexBuffer::~VertexBuffer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    IndexBuffer::IndexBuffer(const IndexBufferDescription& description, const char* debugName)
        : m_Description(description)
    {
        auto d3dDevice = Renderer::GetDevice()->GetD3DDevice();

        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(GetSize());

        DXCall(d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_D3DResource)));

        // If we supply any data, create an upload resource
        if (description.Data)
        {
            ComPtr<ID3D12Resource> uploadBuffer = nullptr;

            DXCall(d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

            D3D12_SUBRESOURCE_DATA indexData = {};
            indexData.pData = description.Data;
            indexData.RowPitch = GetSize();
            indexData.SlicePitch = indexData.RowPitch;

            Ref<CommandBuffer> commandBuffer = CreateRef<CommandBuffer>("CopyCommandBuffer");
            commandBuffer->Begin();
            auto commandList = commandBuffer->GetCommandList();

            UpdateSubresources<1>(commandList.Get(), m_D3DResource.Get(), uploadBuffer.Get(), 0, 0, 1, &indexData);

            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_D3DResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

            commandBuffer->End();
            u64 fenceValue = Renderer::GetDevice()->GetCommandQueue(CommandQueueType::Graphics)->ExecuteCommandList(commandBuffer.get());
            Renderer::GetDevice()->GetCommandQueue(CommandQueueType::Graphics)->WaitForFenceValue(fenceValue);
        }

        // Create buffer view
        m_View.BufferLocation = m_D3DResource->GetGPUVirtualAddress();
        m_View.SizeInBytes = GetSize();
        m_View.Format = Utils::AtomIndexBufferFormatToD3D12(m_Description.Format);

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        ResourceStateTracker::AddGlobalResourceState(m_D3DResource.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    IndexBuffer::~IndexBuffer()
    {
    }
}
