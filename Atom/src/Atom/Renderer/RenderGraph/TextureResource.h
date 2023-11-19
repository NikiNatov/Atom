#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/RenderGraph/Resource.h"

namespace Atom
{
    class TextureResource : public Resource
    {
    public:
        using HWResourceType = Texture;
        using ResourceDescType = TextureDescription;

        struct ViewData
        {
            HWResourceType*  MainResource = nullptr;
            HWResourceType** MipViews = nullptr;
            HWResourceType** SliceViews = nullptr;
            HWResourceType** SubresourceViews = nullptr;
        };

    public:
        TextureResource(const ResourceID_UA& id, const ResourceDescType& description);
        TextureResource(const ResourceID_UA& id, HWResourceType* externalResource);
        ~TextureResource();

        virtual void Allocate() override;
        virtual void Free() override;
        virtual bool IsAllocated() const override;
        virtual bool CanDecayToCommonStateFromState(ResourceState state) const override;
        virtual bool CanPromoteFromCommonStateToState(ResourceState state) const override;
        virtual HWResource* GetHWResource() const override;

        HWResourceType* GetView(u32 mip, u32 slice) const;

        inline TextureFormat GetFormat() const { return m_Description.Format; }
        inline u32 GetWidth() const { return m_Description.Width; }
        inline u32 GetHeight() const { return m_Description.Height; }
        inline u32 GetDepth() const { return m_Description.Depth; }
        inline u32 GetArraySize() const { return m_Description.ArraySize; }
        inline u32 GetMipLevels() const { return m_Description.MipLevels; }
        inline TextureFlags GetFlags() const { return m_Description.Flags; }
        inline ResourceState GetInitialState() const { return m_Description.InitialState; }
        inline const ResourceDescType& GetDescription() const { return m_Description; }
    private:
        ResourceDescType m_Description;
        HWResourceType*  m_ExternalResource;
        ViewData*        m_ViewData;
    };
}