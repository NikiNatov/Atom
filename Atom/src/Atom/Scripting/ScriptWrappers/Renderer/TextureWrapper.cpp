#include "atompch.h"
#include "TextureWrapper.h"

#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        // -------------------------------------------------------- Texture ------------------------------------------------------------
        // -----------------------------------------------------------------------------------------------------------------------------
        Texture::Texture(TextureType type, u32 width, u32 height, TextureFormat format, s32 mipLevels)
        {
            TextureDescription textureDesc;
            textureDesc.Width = width;
            textureDesc.Height = height;
            textureDesc.Format = format;
            textureDesc.MipLevels = mipLevels == -1 ? Atom::Texture::CalculateMaxMipCount(width, height) : mipLevels;
            textureDesc.UsageFlags = TextureBindFlags::UnorderedAccess;

            if (type == TextureType::Texture2D)
            {
                Vector<Vector<byte>> emptyData;
                emptyData.resize(textureDesc.MipLevels);

                Ref<Atom::Texture2D> texture = CreateRef<Atom::Texture2D>(textureDesc, emptyData, true);
                AssetManager::RegisterAsset(texture);
                m_Texture = texture;
            }
            else if (type == TextureType::TextureCube)
            {
                Vector<Vector<byte>> emptyCubeData[6];

                for (u32 face = 0; face < 6; face++)
                    emptyCubeData[face].resize(textureDesc.MipLevels);

                Ref<Atom::TextureCube> texture = CreateRef<Atom::TextureCube>(textureDesc, emptyCubeData, true);
                AssetManager::RegisterAsset(texture);
                m_Texture = texture;
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Texture::Texture(const Ref<Atom::Texture>& texture)
            : m_Texture(texture)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Texture::SetFilter(TextureFilter filter)
        {
            if (m_Texture)
            {
                m_Texture->SetFilter(filter);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Texture::SetWrap(TextureWrap wrap)
        {
            if (m_Texture)
            {
                m_Texture->SetWrap(wrap);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        u32 Texture::GetWidth() const
        {
            if (m_Texture)
            {
                return m_Texture->GetWidth();
            }

            return 0;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        u32 Texture::GetHeight() const
        {
            if (m_Texture)
            {
                return m_Texture->GetHeight();
            }

            return 0;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureFormat Texture::GetFormat() const
        {
            if (m_Texture)
            {
                return m_Texture->GetFormat();
            }

            return TextureFormat::None;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        u32 Texture::GetMipLevels() const
        {
            if (m_Texture)
            {
                return m_Texture->GetMipLevels();
            }

            return 0;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureFilter Texture::GetFilter() const
        {
            if (m_Texture)
            {
                return m_Texture->GetFilter();
            }

            return TextureFilter::None;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureWrap Texture::GetWrap() const
        {
            if (m_Texture)
            {
                return m_Texture->GetWrap();
            }

            return TextureWrap::None;
        }

        // -------------------------------------------------------- Texture2D ----------------------------------------------------------
        // -----------------------------------------------------------------------------------------------------------------------------
        Texture2D::Texture2D(u32 width, u32 height, TextureFormat format, s32 mipLevels)
            : Texture(TextureType::Texture2D, width, height, format, mipLevels)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Texture2D::Texture2D(u64 assetUUID)
            : Texture(nullptr)
        {
            if(assetUUID != 0)
                m_Texture = AssetManager::GetAsset<Atom::Texture2D>(assetUUID, true);
        }
        
        // -----------------------------------------------------------------------------------------------------------------------------
        Texture2D::Texture2D(const Ref<Atom::Texture2D>& texture)
            : Texture(texture)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Texture2D::UpdateGPUData(bool makeNonReadable)
        {
            if (m_Texture)
            {
                GetTexture()->UpdateGPUData(makeNonReadable);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Texture2D::IsReadable() const
        {
            if (m_Texture)
            {
                return GetTexture()->IsReadable();
            }

            return false;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Texture2D::SetPixels(const Vector<byte>& pixels, u32 mipLevel)
        {
            if (m_Texture)
            {
                GetTexture()->SetPixels(pixels, mipLevel);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const Vector<byte>& Texture2D::GetPixels(u32 mipLevel)
        {
            if (m_Texture)
            {
                return GetTexture()->GetPixels(mipLevel);
            }

            return {};
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        UUID Texture2D::GetUUID() const
        {
            return m_Texture ? GetTexture()->GetUUID() : 0;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Ref<Atom::Texture2D> Texture2D::GetTexture() const
        {
            return std::dynamic_pointer_cast<Atom::Texture2D>(m_Texture);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Texture2D Texture2D::Find(const std::filesystem::path& assetPath)
        {
            UUID uuid = AssetManager::GetUUIDForAssetPath(AssetManager::GetAssetsFolder() / assetPath);

            if (uuid == 0)
                return Texture2D(nullptr);

            return Texture2D(AssetManager::GetAsset<Atom::Texture2D>(uuid, true));
        }

        // ------------------------------------------------------- TextureCube ---------------------------------------------------------
        // -----------------------------------------------------------------------------------------------------------------------------
        TextureCube::TextureCube(u32 size, TextureFormat format, s32 mipLevels)
            : Texture(TextureType::TextureCube, size, size, format, mipLevels)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureCube::TextureCube(u64 assetUUID)
            : Texture(nullptr)
        {
            if (assetUUID != 0)
                m_Texture = AssetManager::GetAsset<Atom::TextureCube>(assetUUID, true);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureCube::TextureCube(const Ref<Atom::TextureCube>& texture)
            : Texture(texture)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void TextureCube::UpdateGPUData(bool makeNonReadable)
        {
            if (m_Texture)
            {
                GetTexture()->UpdateGPUData(makeNonReadable);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool TextureCube::IsReadable() const
        {
            if (m_Texture)
            {
                return GetTexture()->IsReadable();
            }

            return false;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void TextureCube::SetPixels(const Vector<byte>& pixels, u32 cubeFace, u32 mipLevel)
        {
            if (m_Texture)
            {
                GetTexture()->SetPixels(pixels, cubeFace, mipLevel);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const Vector<byte>& TextureCube::GetPixels(u32 cubeFace, u32 mipLevel)
        {
            if (m_Texture)
            {
                return GetTexture()->GetPixels(cubeFace, mipLevel);
            }

            return {};
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        UUID TextureCube::GetUUID() const
        {
            return m_Texture ? GetTexture()->GetUUID() : 0;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Ref<Atom::TextureCube> TextureCube::GetTexture() const
        {
            return std::dynamic_pointer_cast<Atom::TextureCube>(m_Texture);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureCube TextureCube::Find(const std::filesystem::path& assetPath)
        {
            UUID uuid = AssetManager::GetUUIDForAssetPath(AssetManager::GetAssetsFolder() / assetPath);

            if (uuid == 0)
                return TextureCube(nullptr);

            return TextureCube(AssetManager::GetAsset<Atom::TextureCube>(uuid, true));
        }
    }
}