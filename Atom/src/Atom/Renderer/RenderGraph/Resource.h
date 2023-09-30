#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/ResourceState.h"
#include "Atom/Renderer/HWResource.h"

#include "Atom/Renderer/RenderGraph/ResourceID.h"

namespace Atom
{
    class Resource
    {
    public:
        virtual ~Resource() = default;

        virtual void Allocate() = 0;
        virtual void Free() = 0;
        virtual bool IsAllocated() const = 0;
        virtual bool CanDecayToCommonStateFromState(ResourceState state) const = 0;
        virtual bool CanPromoteFromCommonStateToState(ResourceState state) const = 0;
        virtual HWResource* GetHWResource() const = 0;

        inline void SetProducerPassID(u16 producerPassID) { m_ProducerPassID = producerPassID; }
        inline u16 GetProducerPassID() const { return m_ProducerPassID; }
        inline ResourceID GetID() const { return m_ID; }
        inline const char* GetName() const { return m_ID.GetName(); }

        template<typename T>
        T* As() { return dynamic_cast<T*>(this); }
    protected:
        Resource(ResourceID id, bool external);
    protected:
        ResourceID    m_ID;
        bool          m_External;
        u16           m_ProducerPassID;
    };
}