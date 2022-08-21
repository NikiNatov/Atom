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
    // -----------------------------------------------------------------------------------------------------------------------------
    Buffer::Buffer(BufferType type, const BufferDescription& description, const char* debugName)
        : m_Type(type), m_Description(description)
    {
        auto d3dDevice = Device::Get().GetD3DDevice();

        CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(GetSize());
        D3D12_RESOURCE_STATES initialState = m_Description.IsDynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COMMON;

        auto heapProps = CD3DX12_HEAP_PROPERTIES(m_Description.IsDynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
        DXCall(d3dDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialState, nullptr, IID_PPV_ARGS(&m_D3DResource)));

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
    void* Buffer::Map(u32 rangeBegin, u32 rangeEnd)
    {
        ATOM_ENGINE_ASSERT(m_Description.IsDynamic, "Buffer is not dynamic!");

        m_MapRange = { rangeBegin, rangeEnd };
        void* data = nullptr;
        DXCall(m_D3DResource->Map(0, &m_MapRange, &data));
        return data;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Buffer::Unmap()
    {
        m_D3DResource->Unmap(0, &m_MapRange);
        m_MapRange = {};
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
    bool Buffer::IsDynamic() const
    {
        return m_Description.IsDynamic;
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

    // -----------------------------------------------------------------------------------------------------------------------------
    ConstantBuffer::ConstantBuffer(const BufferDescription& description, const char* debugName)
        : Buffer(BufferType::ConstantBuffer, description, debugName)
    {
        CreateViews();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ConstantBuffer::~ConstantBuffer()
    {
        Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_CBVDescriptor, true);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void ConstantBuffer::CreateViews()
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = m_D3DResource->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = GetSize();

        m_CBVDescriptor = Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
        Device::Get().GetD3DDevice()->CreateConstantBufferView(&cbvDesc, m_CBVDescriptor);
    }
}
