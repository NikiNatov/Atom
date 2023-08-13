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
        D3D12_RESOURCE_STATES initialState;
        CD3DX12_HEAP_PROPERTIES heapProps;

        if (m_Type == BufferType::ReadbackBuffer)
        {
            initialState = D3D12_RESOURCE_STATE_COPY_DEST;
            heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
        }
        else
        {
            initialState = m_Description.IsDynamic ? D3D12_RESOURCE_STATE_GENERIC_READ : Utils::AtomResourceStateToD3D12(description.InitialState);
            heapProps = CD3DX12_HEAP_PROPERTIES(m_Description.IsDynamic ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT);
        }

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
    Buffer::Buffer(Buffer&& rhs) noexcept
        : m_D3DResource(rhs.m_D3DResource), m_Description(rhs.m_Description), m_Type(rhs.m_Type), m_MapRange(rhs.m_MapRange)
    {
        rhs.m_D3DResource = nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Buffer& Buffer::operator=(Buffer&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Device::Get().ReleaseResource(m_D3DResource.Detach(), true);

            m_D3DResource = rhs.m_D3DResource;
            m_Description = rhs.m_Description;
            m_Type = rhs.m_Type;
            m_MapRange = rhs.m_MapRange;

            rhs.m_D3DResource = nullptr;
        }

        return *this;
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
    ResourceState Buffer::GetInitialState() const
    {
        return m_Description.InitialState;
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
    VertexBuffer::VertexBuffer(VertexBuffer&& rhs) noexcept
        : Buffer(std::move(rhs)), m_View(rhs.m_View)
    {
        rhs.m_View = {};
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Buffer::operator=(std::move(rhs));

            m_View = rhs.m_View;
            rhs.m_View = {};
        }

        return *this;
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
    IndexBuffer::IndexBuffer(IndexBuffer&& rhs) noexcept
        : Buffer(std::move(rhs)), m_View(rhs.m_View), m_Format(rhs.m_Format)
    {
        rhs.m_View = {};
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Buffer::operator=(std::move(rhs));

            m_View = rhs.m_View;
            m_Format = rhs.m_Format;

            rhs.m_View = {};
        }

        return *this;
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
    ConstantBuffer::ConstantBuffer(ConstantBuffer&& rhs) noexcept
        : Buffer(std::move(rhs)), m_CBVDescriptor(rhs.m_CBVDescriptor)
    {
        rhs.m_CBVDescriptor = { 0 };
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ConstantBuffer& ConstantBuffer::operator=(ConstantBuffer&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_CBVDescriptor, true);

            Buffer::operator=(std::move(rhs));

            m_CBVDescriptor = rhs.m_CBVDescriptor;
            rhs.m_CBVDescriptor = { 0 };
        }

        return *this;
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

    // -----------------------------------------------------------------------------------------------------------------------------
    StructuredBuffer::StructuredBuffer(const BufferDescription& description, const char* debugName)
        : Buffer(BufferType::StructuredBuffer, description, debugName)
    {
        CreateViews();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    StructuredBuffer::~StructuredBuffer()
    {
        Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, true);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    StructuredBuffer::StructuredBuffer(StructuredBuffer&& rhs) noexcept
        : Buffer(std::move(rhs)), m_SRVDescriptor(rhs.m_SRVDescriptor)
    {
        rhs.m_SRVDescriptor = { 0 };
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    StructuredBuffer& StructuredBuffer::operator=(StructuredBuffer&& rhs) noexcept
    {
        if (this != &rhs)
        {
            Device::Get().GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->ReleaseDescriptor(m_SRVDescriptor, true);

            Buffer::operator=(std::move(rhs));

            m_SRVDescriptor = rhs.m_SRVDescriptor;
            rhs.m_SRVDescriptor = { 0 };
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void StructuredBuffer::CreateViews()
    {
        auto& dx12Device = Device::Get();

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = m_Description.ElementCount;
        srvDesc.Buffer.StructureByteStride = m_Description.ElementSize;

        m_SRVDescriptor = dx12Device.GetCPUDescriptorHeap(DescriptorHeapType::ShaderResource)->AllocateDescriptor();
        dx12Device.GetD3DDevice()->CreateShaderResourceView(m_D3DResource.Get(), &srvDesc, m_SRVDescriptor);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ReadbackBuffer::ReadbackBuffer(const BufferDescription& description, const char* debugName)
        : Buffer(BufferType::ReadbackBuffer, description, debugName)
    {
    }
}
