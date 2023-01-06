#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/Asset.h"
#include "Atom/Asset/AssetManager.h"

#include "Atom/Renderer/Texture.h"

namespace Atom
{
    class AssetSerializer
    {
    public:
        template<typename T>
        static bool Serialize(const std::filesystem::path& filepath, Ref<T> asset);

        template<typename T>
        static Ref<T> Deserialize(const std::filesystem::path& filepath);
        static bool DeserializeMetaData(const std::filesystem::path& filepath, AssetMetaData& assetMetaData);
    private:
        static void SerializeMetaData(std::ofstream& stream, const AssetMetaData& metaData);
        static void DeserializeMetaData(std::ifstream& stream, AssetMetaData& metaData);
    };
}