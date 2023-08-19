#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/RenderGraph/Resource.h"
#include "Atom/Renderer/RenderGraph/RenderSurfaceResource.h"
#include "Atom/Renderer/RenderGraph/TextureResource.h"

namespace Atom
{
#define DEFINE_VIEW_CLASS(viewClassName, resourceType, returnType, viewDataType, readOnly) \
    struct viewClassName \
    { \
        using ResourceType = resourceType; \
        using ReturnType = returnType; \
        using ViewDataType = viewDataType; \
        inline static const char* ViewClassName = #viewClassName; \
        inline static constexpr bool ReadOnly = readOnly; \
    }

    DEFINE_VIEW_CLASS(TextureSRV, TextureResource, Texture, TextureResource::Data, true);
    DEFINE_VIEW_CLASS(TextureUAV, TextureResource, Texture, TextureResource::Data, false);
    DEFINE_VIEW_CLASS(SurfaceRTV, RenderSurfaceResource, RenderSurface, RenderSurfaceResource::Data, false);
    DEFINE_VIEW_CLASS(SurfaceDSV, RenderSurfaceResource, RenderSurface, RenderSurfaceResource::Data, false);

    class IResourceView
    {
    public:
        virtual bool IsReadOnly() const = 0;
        virtual const char* GetName() const = 0;
        virtual ResourceID GetResourceID() const = 0;
    };

    class RenderGraph;

    template<typename ViewClass>
    class ResourceView : public IResourceView
    {
    public:
        ResourceView(const RenderGraph* graph, ResourceID id)
            : m_Graph(graph), m_ResourceID(id) {}

        virtual bool IsReadOnly() const override { return ViewClass::ReadOnly; }
        virtual const char* GetName() const override { return ViewClass::ViewClassName; }
        virtual ResourceID GetResourceID() const override { return m_ResourceID; }

        typename const ViewClass::ReturnType* GetData() const { return GetData(TextureView::AllMips, TextureView::AllSlices); }
        typename const ViewClass::ReturnType* GetData(u32 mip, u32 slice) const;
    private:
        const RenderGraph* m_Graph;
        ResourceID         m_ResourceID;
    };
}