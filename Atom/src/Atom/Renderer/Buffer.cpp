#include "atompch.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"
#include "Atom/Core/Application.h"

#include "Buffer.h"
#include "Device.h"
#include "CommandBuffer.h"
#include "CommandQueue.h"
#include "ResourceStateTracker.h"

namespace Atom
{
    namespace Utils
    {
        D3D12_RESOURCE_STATES GetInitialStateFromBufferType(BufferType type)
        {
            switch (type)
            {
                case BufferType::VertexBuffer:
                case BufferType::ConstantBuffer:
                    return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                case BufferType::IndexBuffer:
                    return D3D12_RESOURCE_STATE_INDEX_BUFFER;
            }

            ATOM_ENGINE_ASSERT(false, "Unsupported buffer type!");
            return D3D12_RESOURCE_STATE_COMMON;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Buffer::Buffer(BufferType type, const BufferDescription& description, const char* debugName)
        : m_Type(type), m_Description(description)
    {
        auto d3dDevice = Device::Get().GetD3DDevice();

        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(GetSize());

        DXCall(d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_D3DResource)));

        D3D12_RESOURCE_STATES initialState = Utils::GetInitialStateFromBufferType(m_Type);

        // If we supply any data, create an upload resource
        if (m_Description.Data)
        {
            ComPtr<ID3D12Resource> uploadBuffer = nullptr;

            DXCall(d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer)));

            D3D12_SUBRESOURCE_DATA data = {};
            data.pData = m_Description.Data;
            data.RowPitch = GetSize();
            data.SlicePitch = data.RowPitch;

            Ref<CommandBuffer> commandBuffer = CreateRef<CommandBuffer>("CopyCommandBuffer");
            commandBuffer->Begin();
            auto commandList = commandBuffer->GetCommandList();

            UpdateSubresources<1>(commandList.Get(), m_D3DResource.Get(), uploadBuffer.Get(), 0, 0, 1, &data);

            commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_D3DResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, initialState));

            commandBuffer->End();
            u64 fenceValue = Device::Get().GetCommandQueue(CommandQueueType::Graphics)->ExecuteCommandList(commandBuffer.get());
            Device::Get().GetCommandQueue(CommandQueueType::Graphics)->WaitForFenceValue(fenceValue);
        }

#if defined (ATOM_DEBUG)
        String name = debugName;
        DXCall(m_D3DResource->SetName(STRING_TO_WSTRING(name).c_str()));
#endif

        ResourceStateTracker::AddGlobalResourceState(m_D3DResource.Get(), initialState);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Buffer::~Buffer()
    {
        Device::Get().ReleaseResource(m_D3DResource.Detach(), true);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    BufferType Buffer::GetType() const
    {
        return m_Type;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Buffer::GetElementSize() const
    {
        return m_Description.ElementSize;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Buffer::GetElementCount() const
    {
        return m_Description.ElementCount;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    u32 Buffer::GetSize() const
    {
        return m_Description.ElementSize * m_Description.ElementCount;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    VertexBuffer::VertexBuffer(const BufferDescription& description, const char* debugName)
        : Buffer(BufferType::VertexBuffer, description, debugName)
    {
        CreateViews();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    VertexBuffer::~VertexBuffer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void VertexBuffer::CreateViews()
    {
        m_View.BufferLocation = m_D3DResource->GetGPUVirtualAddress();
        m_View.SizeInBytes = GetSize();
        m_View.StrideInBytes = m_Description.ElementSize;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    IndexBuffer::IndexBuffer(const BufferDescription& description, IndexBufferFormat format, const char* debugName)
        : Buffer(BufferType::IndexBuffer, description, debugName), m_Format(format)
    {
        CreateViews();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    IndexBuffer::~IndexBuffer()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    IndexBufferFormat IndexBuffer::GetFormat() const
    {
        return m_Format;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void IndexBuffer::CreateViews()
    {
        m_View.BufferLocation = m_D3DResource->GetGPUVirtualAddress();
        m_View.SizeInBytes = GetSize();
        m_View.Format = Utils::AtomIndexBufferFormatToD3D12(m_Format);
    }
}
