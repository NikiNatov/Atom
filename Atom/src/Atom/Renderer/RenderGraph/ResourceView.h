#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/RenderGraph/Resource.h"
#include "Atom/Renderer/RenderGraph/RenderSurfaceResource.h"
#include "Atom/Renderer/RenderGraph/TextureResource.h"

namespace Atom
{
#define DEFINE_VIEW_CLASS(viewClassName, returnType, readOnly) \
    struct viewClassName \
    { \
        using ReturnType = returnType; \
        inline static const char* ViewClassName = #viewClassName; \
        inline static constexpr bool ReadOnly = readOnly; \
    }

    DEFINE_VIEW_CLASS(TextureUAV, Texture, false);
    DEFINE_VIEW_CLASS(TextureSRV, Texture, true);
    DEFINE_VIEW_CLASS(SurfaceRTV, RenderSurface, false);
    DEFINE_VIEW_CLASS(SurfaceDSV, RenderSurface, false);

    class IResourceView
    {
    public:
        virtual bool IsReadOnly() const = 0;
        virtual const char* GetName() const = 0;
        virtual ResourceID GetResourceID() const = 0;

        template<typename ResourceType, typename ViewClass>
        bool IsResourceViewType() { return dynamic_cast<ResourceView<ResourceType, ViewClass>*>(this); }
    };

    class RenderGraph;

    template<typename ResourceType, typename ViewClass>
    class ResourceView : public IResourceView
    {
    public:
        ResourceView(const ResourceType* resource)
            : m_Resource(resource) {}

        virtual bool IsReadOnly() const override { return ViewClass::ReadOnly; }
        virtual const char* GetName() const override { return ViewClass::ViewClassName; }
        virtual ResourceID GetResourceID() const override { return m_Resource->GetID(); }

        typename const ViewClass::ReturnType* GetData() const { return GetData<ViewClass>(TextureView::AllMips, TextureView::AllSlices); }
        typename const ViewClass::ReturnType* GetData(u32 mip, u32 slice) const;
    private:
        const ResourceType* m_Resource;
    };
}