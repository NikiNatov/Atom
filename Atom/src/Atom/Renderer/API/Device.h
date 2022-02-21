#pragma once

#include "Atom/Core/Core.h"

#include "RendererAPI.h"

namespace Atom
{
    class Adapter;
    class DX12Device;

    class Device
    {
    public:
        virtual ~Device() = default;

        virtual const Adapter* GetAdapter() const = 0;
        virtual bool IsRayTracingSupported() const = 0;

        template<typename T>
        T* As() const
        {
            ATOM_ENGINE_ASSERT(false, "Type not supported!");
            return nullptr;
        }

        template<>
        DX12Device* As() const
        {
            ATOM_ENGINE_ASSERT(RendererAPI::GetAPI() == RendererAPI::API::DirectX12, "Cannot cast to DX12Device. DirectX 12 is not the selected API.");
            return (DX12Device*)(this);
        }

        static Scope<Device> CreateDevice(const Adapter* adapter);
    };
}