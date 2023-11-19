#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/RenderGraph/ResourceID.h"
#include "Atom/Renderer/RenderGraph/RenderSurfaceResource.h"
#include "Atom/Renderer/RenderGraph/TextureResource.h"

namespace Atom
{
#define DEFINE_VIEW_CLASS(viewClassName, resourceType, hwResourceType, readOnly, flagsType, flags) \
    struct viewClassName \
    { \
        using ResourceType = resourceType; \
        using HWResourceType = hwResourceType; \
        inline static const char* ViewClassName = #viewClassName; \
        inline static constexpr bool ReadOnly = readOnly; \
        inline static constexpr flagsType RequiredFlags = flags; \
    }

    DEFINE_VIEW_CLASS(TextureUAV, TextureResource, Texture, false, TextureFlags, TextureFlags::UnorderedAccess);
    DEFINE_VIEW_CLASS(TextureSRV, TextureResource, Texture, true, TextureFlags, TextureFlags::ShaderResource);
    DEFINE_VIEW_CLASS(SurfaceRTV, RenderSurfaceResource, RenderSurface, false, TextureFlags, TextureFlags::RenderTarget);
    DEFINE_VIEW_CLASS(SurfaceDSV_RW, RenderSurfaceResource, RenderSurface, false, TextureFlags, TextureFlags::DepthStencil);
    DEFINE_VIEW_CLASS(SurfaceDSV_RO, RenderSurfaceResource, RenderSurface, true, TextureFlags, TextureFlags::DepthStencil);
    DEFINE_VIEW_CLASS(SurfaceSRV, RenderSurfaceResource, Texture, true, TextureFlags, TextureFlags::ShaderResource);

    template<typename ViewClass>
    class ResourceView;

    class IResourceView
    {
    public:
        virtual bool IsReadOnly() const = 0;
        virtual const char* GetName() const = 0;
        virtual const ResourceID& GetResourceID() const = 0;

        template<typename ViewClass>
        ResourceView<ViewClass>* As() { return dynamic_cast<ResourceView<ViewClass>*>(this); }
    };

    class ResourceScheduler;

    template<typename ViewClass>
    class ResourceView : public IResourceView
    {
    public:
        ResourceView(const ResourceID& resourceID, ResourceScheduler& resourceScheduler)
            : m_ResourceID(resourceID), m_ResourceScheduler(resourceScheduler) {}

        virtual bool IsReadOnly() const override { return ViewClass::ReadOnly; }
        virtual const char* GetName() const override { return ViewClass::ViewClassName; }
        virtual const ResourceID& GetResourceID() const override { return m_ResourceID; }

        typename ViewClass::HWResourceType* GetData() const;
        typename ViewClass::HWResourceType* GetData(u32 mip, u32 slice) const;

    private:
        const ResourceID&  m_ResourceID;
        ResourceScheduler& m_ResourceScheduler;
    };
}