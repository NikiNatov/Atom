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
        static bool Serialize(Ref<T> asset);

        template<typename T>
        static Ref<T> Deserialize(const std::filesystem::path& filepath);
        static bool DeserializeMetaData(const std::filesystem::path& filepath, AssetMetaData& assetMetaData);
    private:
        static void SerializeMetaData(std::ofstream& stream, Ref<Asset> asset);
        static void DeserializeMetaData(std::ifstream& stream, UUID& uuid, AssetType& assetType, std::filesystem::path& sourcePath);
    };
}