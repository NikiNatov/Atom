#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/TextureAsset.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        class Texture
        {
        public:
            virtual ~Texture() = default;

            void UpdateGPUData(bool makeNonReadable = false);
            bool IsCpuReadable() const;
            bool IsGpuWritable() const;

            void SetFilter(TextureFilter filter);
            void SetWrap(TextureWrap wrap);

            TextureFormat GetFormat() const;
            u32 GetWidth() const;
            u32 GetHeight() const;
            u32 GetDepth() const;
            u32 GetArraySize() const;
            u32 GetMipLevels() const;
            TextureFilter GetFilter() const;
            TextureWrap GetWrap() const;
            UUID GetUUID() const;
        protected:
            Texture(u32 width, u32 height, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable);
            Texture(u32 cubeSize, TextureFormat format, u32 mipCount, bool cpuReadable, bool gpuWritable);
            Texture(const Ref<Atom::TextureAsset>& texture);
        protected:
            Ref<Atom::TextureAsset> m_TextureAsset = nullptr;
        };

        class Texture2D : public Texture
        {
        public:
            Texture2D(u32 width, u32 height, TextureFormat format, u32 mipLevels, bool cpuReadable, bool gpuWritable);
            Texture2D(u64 assetUUID);
            Texture2D(const Ref<Atom::Texture2D>& texture);

            void SetPixels(const Vector<byte>& pixels, u32 mipLevel = 0);
            const Vector<byte>& GetPixels(u32 mipLevel = 0);
            Ref<Atom::Texture2D> GetTexture() const;

            static Texture2D Find(const std::filesystem::path& assetPath);
        };

        class TextureCube : public Texture
        {
        public:
            TextureCube(u32 size, TextureFormat format, u32 mipLevels, bool cpuReadable, bool gpuWritable);
            TextureCube(u64 assetUUID);
            TextureCube(const Ref<Atom::TextureCube>& texture);

            void SetPixels(const Vector<byte>& pixels, u32 cubeFace, u32 mipLevel = 0);
            const Vector<byte>& GetPixels(u32 cubeFace, u32 mipLevel = 0);
            Ref<Atom::TextureCube> GetTexture() const;

            static TextureCube Find(const std::filesystem::path& assetPath);
        };
    }
}