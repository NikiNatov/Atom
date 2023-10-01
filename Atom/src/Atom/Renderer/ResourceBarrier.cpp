#include "atompch.h"
#include "ResourceBarrier.h"

#include "Atom/Core/DirectX12/DirectX12Utils.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    ResourceBarrier::ResourceBarrier(const HWResource* resource)
        : m_Resource(resource)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    TransitionBarrier::TransitionBarrier(const HWResource* resource, ResourceState beforeState, ResourceState afterState, u32 subresource)
        : ResourceBarrier(resource), m_BeforeState(beforeState), m_AfterState(afterState), m_Subresource(subresource)
    {
        m_D3DBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        m_D3DBarrier.Transition.pResource = m_Resource->GetD3DResource().Get();
        m_D3DBarrier.Transition.StateBefore = Utils::AtomResourceStateToD3D12(m_BeforeState);
        m_D3DBarrier.Transition.StateAfter = Utils::AtomResourceStateToD3D12(m_AfterState);
        m_D3DBarrier.Transition.Subresource = subresource;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TransitionBarrier::SetBeforeState(ResourceState state)
    {
        m_BeforeState = state;
        m_D3DBarrier.Transition.StateBefore = Utils::AtomResourceStateToD3D12(m_BeforeState);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TransitionBarrier::SetAfterState(ResourceState state)
    {
        m_AfterState = state;
        m_D3DBarrier.Transition.StateAfter = Utils::AtomResourceStateToD3D12(m_AfterState);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void TransitionBarrier::SetSubresource(u32 subresource)
    {
        m_Subresource = subresource;
        m_D3DBarrier.Transition.Subresource = m_Subresource;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UAVBarrier::UAVBarrier(const HWResource* resource)
        : ResourceBarrier(resource)
    {
        m_D3DBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        m_D3DBarrier.UAV.pResource = m_Resource->GetD3DResource().Get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    AliasingBarrier::AliasingBarrier(const HWResource* resource, const HWResource* aliasingResource)
        : ResourceBarrier(resource), m_AliasingResource(aliasingResource)
    {
        m_D3DBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
        m_D3DBarrier.Aliasing.pResourceBefore = m_Resource->GetD3DResource().Get();
        m_D3DBarrier.Aliasing.pResourceAfter = m_AliasingResource->GetD3DResource().Get();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AliasingBarrier::SetAliasingResource(const HWResource* resource)
    {
        m_Resource = resource;
        m_D3DBarrier.Aliasing.pResourceAfter = m_AliasingResource->GetD3DResource().Get();
    }
}
