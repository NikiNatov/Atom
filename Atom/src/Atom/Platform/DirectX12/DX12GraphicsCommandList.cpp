#include "atompch.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DX12GraphicsCommandList.h"
#include "DX12Device.h"

namespace Atom
{
    // ----------------------------------------------Command Allocator--------------------------------------------------------------
    DX12CommandAllocator::DX12CommandAllocator(const Device* device, CommandListType commandListType)
        : m_Device(device), m_AllocatorType(commandListType)
    {
        auto d3dDevice = m_Device->As<DX12Device>()->GetD3DDevice();
        DXCall(d3dDevice->CreateCommandAllocator(Utils::AtomCommandListTypeToD3D12(commandListType), IID_PPV_ARGS(&m_D3DAllocator)));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12CommandAllocator::~DX12CommandAllocator()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12CommandAllocator::Reset()
    {
        DXCall(m_D3DAllocator->Reset());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CommandListType DX12CommandAllocator::GetAllocatorType() const
    {
        return m_AllocatorType;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Device* DX12CommandAllocator::GetCreationDevice() const
    {
        return m_Device;
    }

    // ----------------------------------------------Graphics Command List----------------------------------------------------------
    DX12GraphicsCommandList::DX12GraphicsCommandList(const Device* device, CommandListType type, const CommandAllocator* allocator)
        : m_Device(device), m_Allocator(allocator), m_Type(type)
    {
        ATOM_ENGINE_ASSERT(allocator->GetAllocatorType() == m_Type, "Allocator type does not match the command list type!");

        auto d3dDevice = m_Device->As<DX12Device>()->GetD3DDevice();
        auto d3dAllocator = m_Allocator->As<DX12CommandAllocator>()->GetD3DAllocator();
        DXCall(d3dDevice->CreateCommandList(0, Utils::AtomCommandListTypeToD3D12(m_Type), d3dAllocator.Get(), nullptr, IID_PPV_ARGS(&m_D3DGraphicsCommandList)));
        Close();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    DX12GraphicsCommandList::~DX12GraphicsCommandList()
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12GraphicsCommandList::Reset(const CommandAllocator* allocator)
    {
        ATOM_ENGINE_ASSERT(allocator->GetAllocatorType() == m_Type, "Allocator type does not match the command list type!");

        m_Allocator = allocator;
        auto d3dAllocator = m_Allocator->As<DX12CommandAllocator>()->GetD3DAllocator();
        DXCall(m_D3DGraphicsCommandList->Reset(d3dAllocator.Get(), nullptr));
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void DX12GraphicsCommandList::Close()
    {
        DXCall(m_D3DGraphicsCommandList->Close());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const Device* DX12GraphicsCommandList::GetCreationDevice() const
    {
        return m_Device;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const CommandAllocator* DX12GraphicsCommandList::GetAllocator() const
    {
        return m_Allocator;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    CommandListType DX12GraphicsCommandList::GetType() const
    {
        return m_Type;
    }
}

#endif // ATOM_PLATFORM_WINDOWS
