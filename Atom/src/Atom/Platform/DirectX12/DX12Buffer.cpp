#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12Buffer.h"
#include "DX12Device.h"
#include "DX12ResourceStateTracker.h"
#include "DX12CommandBuffer.h"

namespace Atom
{
    // ------------------------------------------------ Vertex Buffer --------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12VertexBuffer::DX12VertexBuffer(const VertexBufferDescription& description, const char* debugName)
        : m_Description(description)
    {
        auto d3dDevice = Renderer::GetDevice().As<DX12Device>()->GetD3DDevice();

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

            Ref<CommandBuffer> commandBuffer = CommandBuffer::Create("CopyCommandBuffer");
            commandBuffer->Begin();
            auto commandList = commandBuffer->As<DX12CommandBuffer>()->GetCommandList();

            UpdateSubresources<1>(commandList.Get(), m_D3DResource.Get(), uploadBuffer.Get(), 0, 0, 1, &vertexData);

            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_D3DResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

            commandBuffer->End();
            u64 fenceValue = Renderer::GetDevice().GetCommandQueue(CommandQueueType::Graphics).ExecuteCommandList(commandBuffer.get());
            Renderer::GetDevice().GetCommandQueue(CommandQueueType::Graphics).WaitForFenceValue(fenceValue);
        }

        // Create buffer view
        m_View.BufferLocation = m_D3DResource->GetGPUVirtualAddress();
        m_View.SizeInBytes = GetSize();
        m_View.StrideInBytes = m_Description.VertexStride;

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        DX12ResourceStateTracker::AddGlobalResourceState(m_D3DResource.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12VertexBuffer::~DX12VertexBuffer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12VertexBuffer::GetVertexCount() const
    {
        return m_Description.VertexCount;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12VertexBuffer::GetVertexStride() const
    {
        return m_Description.VertexStride;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12VertexBuffer::GetSize() const
    {
        return m_Description.VertexCount * m_Description.VertexStride;
    }

    // ------------------------------------------------ Index Buffer ---------------------------------------------------------------
    // -----------------------------------------------------------------------------------------------------------------------------
    DX12IndexBuffer::DX12IndexBuffer(const IndexBufferDescription& description, const char* debugName)
        : m_Description(description), m_DXGIFormat(Utils::AtomIndexBufferFormatToD3D12(m_Description.Format))
    {
        auto d3dDevice = Renderer::GetDevice().As<DX12Device>()->GetD3DDevice();

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

            Ref<CommandBuffer> commandBuffer = CommandBuffer::Create("CopyCommandBuffer");
            commandBuffer->Begin();
            auto commandList = commandBuffer->As<DX12CommandBuffer>()->GetCommandList();

            UpdateSubresources<1>(commandList.Get(), m_D3DResource.Get(), uploadBuffer.Get(), 0, 0, 1, &indexData);

            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_D3DResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

            commandBuffer->End();
            u64 fenceValue = Renderer::GetDevice().GetCommandQueue(CommandQueueType::Graphics).ExecuteCommandList(commandBuffer.get());
            Renderer::GetDevice().GetCommandQueue(CommandQueueType::Graphics).WaitForFenceValue(fenceValue);
        }

        // Create buffer view
        m_View.BufferLocation = m_D3DResource->GetGPUVirtualAddress();
        m_View.SizeInBytes = GetSize();
        m_View.Format = m_DXGIFormat;

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        DX12ResourceStateTracker::AddGlobalResourceState(m_D3DResource.Get(), D3D12_RESOURCE_STATE_INDEX_BUFFER);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12IndexBuffer::~DX12IndexBuffer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12IndexBuffer::GetIndexCount() const
    {
        return m_Description.IndexCount;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    IndexBufferFormat DX12IndexBuffer::GetFormat() const
    {
        return m_Description.Format;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 DX12IndexBuffer::GetSize() const
    {
        return m_Description.IndexCount * (m_Description.Format == IndexBufferFormat::U16 ? sizeof(u16) : sizeof(u32));
    }
}
#endif // ATOM_PLATFORM_WINDOWS