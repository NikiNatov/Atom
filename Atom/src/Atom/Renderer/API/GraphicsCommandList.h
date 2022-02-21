#pragma once

#include "Atom/Core/Core.h"

#include "RendererAPI.h"

namespace Atom
{
    class Device;
    class DX12CommandAllocator;
    class DX12GraphicsCommandList;

    enum class CommandListType
    {
        None    = 0,
        Copy    = 1,
        Compute = 2,
        Direct  = 3
    };

    class CommandAllocator
    {
    public:
        virtual ~CommandAllocator() = default;

        virtual void Reset() = 0;
        virtual CommandListType GetAllocatorType() const = 0;
        virtual const Device* GetCreationDevice() const = 0;

        template<typename T>
        T* As() const
        {
            ATOM_ENGINE_ASSERT(false, "Type not supported!");
            return nullptr;
        }

        template<>
        DX12CommandAllocator* As() const
        {
            ATOM_ENGINE_ASSERT(RendererAPI::GetAPI() == RendererAPI::API::DirectX12, "Cannot cast to DX12CommandAllocator. DirectX 12 is not the selected API.");
            return (DX12CommandAllocator*)(this);
        }

        static Scope<CommandAllocator> CreateCommandAllocator(const Device* device, CommandListType commandListType);
    };

    class GraphicsCommandList
    {
    public:
        virtual ~GraphicsCommandList() = default;

        virtual void Reset(const CommandAllocator* allocator) = 0;
        virtual void Close() = 0;

        virtual const Device* GetCreationDevice() const = 0;
        virtual const CommandAllocator* GetAllocator() const = 0;
        virtual CommandListType GetType() const = 0;

        template<typename T>
        T* As() const
        {
            ATOM_ENGINE_ASSERT(false, "Type not supported!");
            return nullptr;
        }

        template<>
        DX12GraphicsCommandList* As() const
        {
            ATOM_ENGINE_ASSERT(RendererAPI::GetAPI() == RendererAPI::API::DirectX12, "Cannot cast to DX12GraphicsCommandList. DirectX 12 is not the selected API.");
            return (DX12GraphicsCommandList*)(this);
        }

        static Scope<GraphicsCommandList> CreateGraphicsCommandList(const Device* device, CommandListType type, const CommandAllocator* allocator);
    };
}