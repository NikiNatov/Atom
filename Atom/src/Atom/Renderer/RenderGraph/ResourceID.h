#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    class TextureResource;
    class RenderSurfaceResource;

    class ResourceIDRegistry
    {
    public:
        static u16 Register(const char* name);
        static void Unregister(u16 idx);

        static const char* GetName(u16 idx);
        static u32 GetResourceCount();
    private:
        inline static Vector<const char*> ms_Resources;
        inline static Stack<u16> ms_FreeIDs;
    };

    class ResourceID
    {
    public:
        static constexpr u16 InvalidID = 0xffff;
    public:
        ResourceID(const char* name);
        ~ResourceID();

        inline bool IsValid() const { return m_Index != InvalidID; }
        inline u16 GetIndex() const { return m_Index; }
        inline const char* GetName() const { return ResourceIDRegistry::GetName(m_Index); }

        inline bool operator==(const ResourceID& rhs) { return m_Index == rhs.m_Index; }
        inline bool operator!=(const ResourceID& rhs) { return !(*this == rhs); }
        inline operator bool() { return IsValid(); }
        inline operator u16() { return m_Index; }
    private:
        u16 m_Index = InvalidID;
    };

#define DEFINE_RESOURCE_ID_TYPE(typeName, resourceType) \
    class typeName : public ResourceID                  \
    {                                                   \
    public:                                             \
        using ResourceType = resourceType;              \
    public:                                             \
        typeName(const char* name)                      \
            : ResourceID(name) {}                       \
    };

    DEFINE_RESOURCE_ID_TYPE(ResourceID_UA, TextureResource);
    DEFINE_RESOURCE_ID_TYPE(ResourceID_RT, RenderSurfaceResource);
    DEFINE_RESOURCE_ID_TYPE(ResourceID_DS, RenderSurfaceResource);

#define RID(name) __ResourceID_##name

#define DEFINE_RID_UA(name) ResourceID_UA RID(name)(#name)
#define DECLARE_RID_UA(name) extern ResourceID_UA RID(name)

#define DEFINE_RID_RT(name) ResourceID_RT RID(name)(#name)
#define DECLARE_RID_RT(name) extern ResourceID_RT RID(name)

#define DEFINE_RID_DS(name) ResourceID_DS RID(name)(#name)
#define DECLARE_RID_DS(name) extern ResourceID_DS RID(name)
}