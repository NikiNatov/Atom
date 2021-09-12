#pragma once

#include "Atom/Renderer/GraphicsCommandList.h"

#if defined(ATOM_PLATFORM_WINDOWS)

#include "DirectX12.h"

namespace Atom
{
    class DX12CommandAllocator : public CommandAllocator
    {
    public:
        DX12CommandAllocator(const Device* device, CommandListType commandListType);
        ~DX12CommandAllocator();

        virtual void Reset() override;
        virtual CommandListType GetAllocatorType() const override;
        virtual const Device* GetCreationDevice() const override;

        inline wrl::ComPtr<ID3D12CommandAllocator> GetD3DAllocator() const { return m_D3DAllocator; }
    private:
        const Device*                       m_Device;
        CommandListType                     m_AllocatorType;
        wrl::ComPtr<ID3D12CommandAllocator> m_D3DAllocator;
    };

    class DX12GraphicsCommandList : public GraphicsCommandList
    {
    public:
        DX12GraphicsCommandList(const Device* device, CommandListType type, const CommandAllocator* allocator);
        ~DX12GraphicsCommandList();

        virtual void Reset(const CommandAllocator* allocator) override;
        virtual void Close() override;

        virtual const Device* GetCreationDevice() const override;
        virtual const CommandAllocator* GetAllocator() const override;
        virtual CommandListType GetType() const override;

        inline wrl::ComPtr<ID3D12GraphicsCommandList5> GetD3DGraphicsCommandList() const { return m_D3DGraphicsCommandList; }
    private:
        const Device*                           m_Device;
        const CommandAllocator*                 m_Allocator;
        CommandListType                         m_Type;
        wrl::ComPtr<ID3D12GraphicsCommandList5> m_D3DGraphicsCommandList;
        
    };
}

#endif // ATOM_PLATFORM_WINDOWS