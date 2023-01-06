#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"

namespace Atom
{
    struct TextureImportSettings
    {
        TextureType   Type = TextureType::Texture2D;
        u32           CubemapSize = 1024;
        TextureFormat Format = TextureFormat::RGBA8;
        TextureFilter Filter = TextureFilter::Linear;
        TextureWrap   Wrap = TextureWrap::Repeat;
        bool          IsReadable = false;
    };

    struct MeshImportSettings
    {
        bool IsReadable = false;
        bool SmoothNormals = true;
        bool PreserveHierarchy = false;
    };

    class ContentTools
    {
    public:
        static UUID ImportTextureAsset(const std::filesystem::path& sourcePath, const std::filesystem::path& destinationFolder, const TextureImportSettings& importSettings);
        static UUID ImportTextureAsset(const byte* compressedData, u32 dataSize, const String& assetName, const std::filesystem::path& destinationFolder, const TextureImportSettings& importSettings);
        static UUID ImportMeshAsset(const std::filesystem::path& sourcePath, const std::filesystem::path& destinationFolder, const MeshImportSettings& importSettings);
        static Ref<Texture2D> ImportTexture(const std::filesystem::path& sourcePath, const TextureImportSettings& importSettings);
        static Ref<Texture2D> ImportTexture(const byte* compressedData, u32 dataSize, const String& name, const TextureImportSettings& importSettings);
        static UUID CreateMaterialAsset(const std::filesystem::path& filepath = "");
        static UUID CreateSceneAsset(const String& sceneName = "Unnamed Scene", const std::filesystem::path& filepath = "");
    private:
        static bool DecodeImage(const std::filesystem::path& sourcePath, TextureFormat& format, s32& width, s32& height, Vector<byte>& pixels);
        static bool DecodeImage(const byte* compressedData, u32 dataSize, TextureFormat& format, s32& width, s32& height, Vector<byte>& pixels);
    };
}