#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"

#include "Atom/Renderer/HWResource.h"
#include "Atom/Renderer/ResourceState.h"

namespace Atom
{
    class ResourceBarrier
    {
    public:
        virtual ~ResourceBarrier() = default;

        inline const HWResource* GetResource() const { return m_Resource; }
        inline const D3D12_RESOURCE_BARRIER& GetD3DBarrier() const { return m_D3DBarrier; }
    protected:
        ResourceBarrier(const HWResource* resource);
    protected:
        const HWResource* m_Resource = nullptr;
        D3D12_RESOURCE_BARRIER m_D3DBarrier{};
    };

    class TransitionBarrier : public ResourceBarrier
    {
    public:
        TransitionBarrier(const HWResource* resource, ResourceState beforeState, ResourceState afterState);

        inline ResourceState GetBeforeState() const { return m_BeforeState; }
        inline ResourceState GetAfterState() const { return m_AfterState; }
    private:
        ResourceState m_BeforeState;
        ResourceState m_AfterState;
    };

    class UAVBarrier : public ResourceBarrier
    {
    public:
        UAVBarrier(const HWResource* resource);
    };

    class AliasingBarrier : public ResourceBarrier
    {
    public:
        AliasingBarrier(const HWResource* resource, const HWResource* aliasingResource);

        inline const HWResource* GetAliasingResource() const { return m_AliasingResource; }
    private:
        const HWResource* m_AliasingResource = nullptr;
    };
}