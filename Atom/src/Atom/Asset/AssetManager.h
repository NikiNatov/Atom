#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/UUID.h"
#include "Atom/Asset/Asset.h"

#include <FileWatch.h>

namespace Atom
{
    class AssetManager
    {
    public:
        static void Shutdown();
        static void SetAssetFolder(const std::filesystem::path& assetFolder);
        static void RegisterAsset(const AssetMetaData& metaData);
        static void RegisterAsset(const Ref<Asset>& asset);
        static void RegisterAsset(const std::filesystem::path& assetPath);
        static void UnregisterAsset(UUID uuid);
        static void UnregisterAsset(const std::filesystem::path& assetPath);
        static bool LoadAsset(UUID uuid);
        static bool ReloadAsset(UUID uuid);
        static void UnloadAsset(UUID uuid);
        static void UnloadAllAssets();
        static void UnloadUnusedAssets();
        static bool IsAssetValid(UUID uuid);
        static bool IsAssetLoaded(UUID uuid);
        static const AssetMetaData* GetAssetMetaData(UUID uuid);
        static u32 GetAssetRefCount(UUID uuid);
        static const HashMap<UUID, AssetMetaData>& GetRegistry();
        static UUID GetUUIDForAssetPath(const std::filesystem::path& assetPath);
        static const std::filesystem::path& GetAssetsFolder();
        static bool IsAssetFile(const std::filesystem::path& filepath);

        template<typename T>
        static Ref<T> GetAsset(UUID uuid, bool load = false)
        {
            if (load && !LoadAsset(uuid))
                return nullptr;

            if(!IsAssetLoaded(uuid))
                return nullptr;

            return std::dynamic_pointer_cast<T>(ms_LoadedAssets[uuid]);
        }

    private:
        static void RegisterAllAssets(const std::filesystem::path& assetFolder);
    private:
        inline static std::filesystem::path                              ms_AssetsFolder;
        inline static HashMap<UUID, AssetMetaData>                       ms_Registry;
        inline static HashMap<std::filesystem::path, UUID>               ms_AssetPathUUIDs;
        inline static HashMap<UUID, Ref<Asset>>                          ms_LoadedAssets;
        inline static HashMap<UUID, bool>                                ms_PendingReloads;
        inline static Scope<filewatch::FileWatch<std::filesystem::path>> ms_FileWatcher;
    };
}