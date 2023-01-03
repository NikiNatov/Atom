#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        class Texture
        {
        public:
            virtual ~Texture() = default;

            void SetFilter(TextureFilter filter);
            void SetWrap(TextureWrap wrap);

            u32 GetWidth() const;
            u32 GetHeight() const;
            TextureFormat GetFormat() const;
            u32 GetMipLevels() const;
            TextureFilter GetFilter() const;
            TextureWrap GetWrap() const;

        protected:
            Texture(TextureType type, u32 width, u32 height, TextureFormat format, s32 mipLevels = -1);
            Texture(const Ref<Atom::Texture>& texture);
        protected:
            Ref<Atom::Texture> m_Texture = nullptr;
        };

        class Texture2D : public Texture
        {
        public:
            Texture2D(u32 width, u32 height, TextureFormat format, s32 mipLevels = -1);
            Texture2D(const Ref<Atom::Texture2D>& texture);

            void UpdateGPUData(bool makeNonReadable = false);
            bool IsReadable() const;

            void SetPixels(const Vector<byte>& pixels, u32 mipLevel = 0);
            const Vector<byte>& GetPixels(u32 mipLevel = 0);

            Ref<Atom::Texture2D> GetTexture() const;

            static Texture2D Find(const std::filesystem::path& assetPath);
        };

        class TextureCube : public Texture
        {
        public:
            TextureCube(u32 size, TextureFormat format, s32 mipLevels = -1);
            TextureCube(const Ref<Atom::TextureCube>& texture);

            void UpdateGPUData(bool makeNonReadable = false);
            bool IsReadable() const;

            void SetPixels(const Vector<byte>& pixels, u32 cubeFace, u32 mipLevel = 0);
            const Vector<byte>& GetPixels(u32 cubeFace, u32 mipLevel = 0);

            Ref<Atom::TextureCube> GetTexture() const;

            static TextureCube Find(const std::filesystem::path& assetPath);
        };
    }
}