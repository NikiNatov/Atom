#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/RenderSurface.h"
#include "Atom/Renderer/RenderGraph/Resource.h"

namespace Atom
{
    class RenderSurfaceResource : public Resource
    {
    public:
        struct Data : DataBase
        {
            RenderSurface*  MainResource;
            RenderSurface** SubresourceViews;
        };
    public:
        RenderSurfaceResource(ResourceID_RT id, const TextureDescription& description);
        RenderSurfaceResource(ResourceID_DS id, const TextureDescription& description);
        RenderSurfaceResource(ResourceID_RT id, RenderSurface* externalSurface);
        RenderSurfaceResource(ResourceID_DS id, RenderSurface* externalSurface);
        ~RenderSurfaceResource();

        virtual void Allocate() override;
        virtual void Free() override;

        inline TextureFormat GetFormat() const { return m_Description.Format; }
        inline u32 GetWidth() const { return m_Description.Width; }
        inline u32 GetHeight() const { return m_Description.Height; }
        inline u32 GetDepth() const { return m_Description.Depth; }
        inline u32 GetArraySize() const { return m_Description.ArraySize; }
        inline u32 GetMipLevels() const { return m_Description.MipLevels; }
        inline TextureFlags GetFlags() const { return m_Description.Flags; }
        inline ResourceState GetInitialState() const { return m_Description.InitialState; }
        inline const TextureDescription& GetDescription() const { return m_Description; }
    public:
        static RenderSurface* GetView(const Data* data, u32 mip, u32 slice);
    private:
        TextureDescription m_Description;
        RenderSurface*     m_ExternalSurface;
    };
}