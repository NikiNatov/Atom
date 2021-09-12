#pragma once

#include "Atom/Core/Core.h"

#include "GraphicsCommandList.h"
#include "RendererAPI.h"

namespace Atom
{
    class Device;
    class DX12CommandQueue;

    enum class CommandQueuePriority
    {
        None    = 0,
        Normal  = 1,
        High    = 2
    };

    struct CommandQueueDesc
    {
        CommandListType      Type = CommandListType::Direct;
        CommandQueuePriority Priority = CommandQueuePriority::Normal;

        CommandQueueDesc()
        {}

        CommandQueueDesc(CommandListType type, CommandQueuePriority priority)
            : Type(type), Priority(priority)
        {}
    };

    class CommandQueue
    {
    public:
        virtual ~CommandQueue() = default;

        virtual u64 Signal() = 0;
        virtual void WaitForFenceValue(u64 value) = 0;
        virtual void Flush() = 0;

        virtual void ExecuteCommandList(const GraphicsCommandList* commandList) = 0;
        virtual void ExecuteCommandLists(const Vector<GraphicsCommandList*>& commandLists) = 0;

        virtual CommandListType GetQueueType() const = 0;
        virtual CommandQueuePriority GetQueuePriority() const = 0;
        virtual const Device* GetCreationDevice() const = 0;
        virtual u64 GetCurrentFenceValue() const = 0;

        template<typename T>
        T* As() const
        {
            ATOM_ENGINE_ASSERT(false, "Type not supported!");
            return nullptr;
        }

        template<>
        DX12CommandQueue* As() const
        {
            ATOM_ENGINE_ASSERT(RendererAPI::GetAPI() == RendererAPI::API::DirectX12, "Cannot cast to DX12CommandQueue. DirectX 12 is not the selected API.");
            return (DX12CommandQueue*)(this);
        }

        static Scope<CommandQueue> CreateCommandQueue(const Device* device, const CommandQueueDesc& description);
    };

}