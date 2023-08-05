#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/RenderGraph/Resource.h"

namespace Atom
{
    class TextureResource : public Resource
    {
    public:
        struct Data : DataBase
        {
            Texture*  MainResource = nullptr;
            Texture** MipViews = nullptr;
            Texture** SliceViews = nullptr;
            Texture** SubresourceViews = nullptr;
        };

    public:
        TextureResource(ResourceID_UA id, const TextureDescription& description);
        TextureResource(ResourceID_UA id, Texture* externalTexture);
        ~TextureResource();

        virtual void Allocate() override;
        virtual void Free() override;

        inline TextureFormat GetFormat() const { return m_Description.Format; }
        inline u32 GetWidth() const { return m_Description.Width; }
        inline u32 GetHeight() const { return m_Description.Height; }
        inline u32 GetDepth() const { return m_Description.Depth; }
        inline u32 GetArraySize() const { return m_Description.ArraySize; }
        inline u32 GetMipLevels() const { return m_Description.MipLevels; }
        inline TextureFlags GetFlags() const { return m_Description.Flags; }
        inline const TextureDescription& GetDescription() const { return m_Description; }
    public:
        static Texture* GetView(const Data* data, u32 mip, u32 slice);
    private:
        TextureDescription m_Description;
        Texture*           m_ExternalTexture;
    };
}