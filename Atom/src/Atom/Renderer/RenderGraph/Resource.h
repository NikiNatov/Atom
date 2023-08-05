#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/RenderGraph/ResourceID.h"

namespace Atom
{
    class Resource
    {
    public:
        struct DataBase {};
    public:
        virtual ~Resource() = default;

        virtual void Allocate() = 0;
        virtual void Free() = 0;

        inline bool IsAllocated() const { return m_Data != nullptr; }
        inline void SetProducerPassID(u16 producerPassID) { m_ProducerPassID = producerPassID; }
        inline u16 GetProducerPassID() const { return m_ProducerPassID; }
        inline ResourceID GetID() const { return m_ID; }
        inline const char* GetName() const { return m_ID.GetName(); }
        inline const DataBase* GetData() const { return m_Data; }
    protected:
        Resource(ResourceID id, bool external);
    protected:
        ResourceID m_ID;
        bool       m_External;
        u16        m_ProducerPassID;
        DataBase*  m_Data;
    };
}