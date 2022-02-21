#pragma once

#include "Atom/Core/Core.h"

#include "RendererAPI.h"

namespace Atom
{
    class DX12Adapter;

    enum class AdapterPreference
    {
        None            = 0,
        MinimumPowered  = 1,
        HighPerformance = 2
    };

    class Adapter
    {
    public:
        virtual ~Adapter() = default;

        virtual u64 GetSystemMemory() const = 0;
        virtual u64 GetVideoMemory() const = 0;
        virtual u64 GetSharedMemory() const = 0;
        virtual String GetDescription() const = 0;

        template<typename T>
        T* As() const
        {
            ATOM_ENGINE_ASSERT(false, "Type not supported!");
            return nullptr;
        }

        template<>
        DX12Adapter* As() const
        {
            ATOM_ENGINE_ASSERT(RendererAPI::GetAPI() == RendererAPI::API::DirectX12, "Cannot cast to DX12Adapter. DirectX 12 is not the selected API.");
            return (DX12Adapter*)(this);
        }

        static Scope<Adapter> CreateAdapter(AdapterPreference preference);
    };
}