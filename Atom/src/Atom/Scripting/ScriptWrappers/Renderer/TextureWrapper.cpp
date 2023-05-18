#include "atompch.h"
#include "TextureWrapper.h"

#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        // -------------------------------------------------------- Texture ------------------------------------------------------------
        // -----------------------------------------------------------------------------------------------------------------------------
        Texture::Texture(u32 width, u32 height, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable)
        {
            m_TextureAsset = CreateRef<Atom::Texture2D>(width, height, format, mipCount, cpuReadable, gpuWritable);
            AssetManager::RegisterAsset(m_TextureAsset);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Texture::Texture(u32 cubeSize, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable)
        {
            m_TextureAsset = CreateRef<Atom::TextureCube>(cubeSize, format, mipCount, cpuReadable, gpuWritable);
            AssetManager::RegisterAsset(m_TextureAsset);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Texture::Texture(const Ref<Atom::TextureAsset>& texture)
            : m_TextureAsset(texture)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Texture::UpdateGPUData(bool makeNonReadable)
        {
            m_TextureAsset->UpdateGPUData(makeNonReadable);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Texture::IsCpuReadable() const
        {
            return m_TextureAsset->IsCpuReadable();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Texture::IsGpuWritable() const
        {
            return m_TextureAsset->IsGpuWritable();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Texture::SetFilter(TextureFilter filter)
        {
            m_TextureAsset->SetFilter(filter);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Texture::SetWrap(TextureWrap wrap)
        {
            m_TextureAsset->SetWrap(wrap);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureFormat Texture::GetFormat() const
        {
            return m_TextureAsset->GetFormat();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        u32 Texture::GetWidth() const
        {
            return m_TextureAsset->GetWidth();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        u32 Texture::GetHeight() const
        {
            return m_TextureAsset->GetHeight();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        u32 Texture::GetDepth() const
        {
            return m_TextureAsset->GetDepth();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        u32 Texture::GetArraySize() const
        {
            return m_TextureAsset->GetArraySize();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        u32 Texture::GetMipLevels() const
        {
            return m_TextureAsset->GetMipLevels();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureFilter Texture::GetFilter() const
        {
            return m_TextureAsset->GetFilter();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureWrap Texture::GetWrap() const
        {
            return m_TextureAsset->GetWrap();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        UUID Texture::GetUUID() const
        {
            return m_TextureAsset->GetUUID();
        }

        // -------------------------------------------------------- Texture2D ----------------------------------------------------------
        // -----------------------------------------------------------------------------------------------------------------------------
        Texture2D::Texture2D(u32 width, u32 height, TextureFormat format, u32 mipLevels, bool cpuReadable, bool gpuWritable)
            : Texture(width, height, format, mipLevels, cpuReadable, gpuWritable)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Texture2D::Texture2D(u64 assetUUID)
            : Texture(nullptr)
        {
            if(assetUUID != 0)
                m_TextureAsset = AssetManager::GetAsset<Atom::Texture2D>(assetUUID, true);
        }
        
        // -----------------------------------------------------------------------------------------------------------------------------
        Texture2D::Texture2D(const Ref<Atom::Texture2D>& texture)
            : Texture(texture)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Texture2D::SetPixels(const Vector<byte>& pixels, u32 mipLevel)
        {
            GetTexture()->SetPixels(pixels, mipLevel);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const Vector<byte>& Texture2D::GetPixels(u32 mipLevel)
        {
            return GetTexture()->GetPixels(mipLevel);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Ref<Atom::Texture2D> Texture2D::GetTexture() const
        {
            return std::static_pointer_cast<Atom::Texture2D>(m_TextureAsset);
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
        TextureCube::TextureCube(u32 size, TextureFormat format, u32 mipLevels, bool cpuReadable, bool gpuWritable)
            : Texture(size, format, mipLevels, cpuReadable, gpuWritable)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureCube::TextureCube(u64 assetUUID)
            : Texture(nullptr)
        {
            if (assetUUID != 0)
                m_TextureAsset = AssetManager::GetAsset<Atom::TextureCube>(assetUUID, true);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        TextureCube::TextureCube(const Ref<Atom::TextureCube>& texture)
            : Texture(texture)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void TextureCube::SetPixels(const Vector<byte>& pixels, u32 cubeFace, u32 mipLevel)
        {
            GetTexture()->SetPixels(pixels, cubeFace, mipLevel);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const Vector<byte>& TextureCube::GetPixels(u32 cubeFace, u32 mipLevel)
        {
            return GetTexture()->GetPixels(cubeFace, mipLevel);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Ref<Atom::TextureCube> TextureCube::GetTexture() const
        {
            return std::static_pointer_cast<Atom::TextureCube>(m_TextureAsset);
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