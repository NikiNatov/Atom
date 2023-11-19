#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    class TextureResource;
    class RenderSurfaceResource;

    class ResourceID
    {
    public:
        static constexpr u16 InvalidIndex = 0xffff;

        struct Hash
        {
            inline size_t operator()(const ResourceID& id) const { return std::hash<u16>()(id.m_Index); }
        };
    public:
        ResourceID() = default;
        ResourceID(const char* name);
        ~ResourceID();

        ResourceID(const ResourceID&) = delete;
        ResourceID& operator=(const ResourceID&) = delete;

        inline bool IsValid() const { return m_Index != InvalidIndex; }
        inline u16 GetIndex() const { return m_Index; }
        inline const char* GetName() const { return m_Name; }

        inline bool operator==(const ResourceID& rhs) const { return m_Index == rhs.m_Index; }
        inline bool operator!=(const ResourceID& rhs) const { return !(*this == rhs); }
        inline bool operator<(const ResourceID& rhs) const { return m_Index < rhs.m_Index; }
        inline operator bool() const { return IsValid(); }
        inline operator u16() const { return m_Index; }
    private:
        u16         m_Index = InvalidIndex;
        const char* m_Name = nullptr;
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