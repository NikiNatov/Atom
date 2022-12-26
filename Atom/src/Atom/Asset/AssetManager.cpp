#include "atompch.h"
#include "AssetManager.h"

#include "Atom/Asset/AssetSerializer.h"
#include "Atom/Core/Application.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/Mesh.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::Shutdown()
    {
        ms_Registry.clear();
        ms_AssetPathUUIDs.clear();
        ms_LoadedAssets.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::SetAssetFolder(const std::filesystem::path& assetFolder)
    {
        ms_Registry.clear();
        ms_AssetPathUUIDs.clear();
        ms_LoadedAssets.clear();
        ms_AssetsFolder = assetFolder;

        RegisterAllAssets(ms_AssetsFolder);

        ms_FileWatcher = CreateScope<filewatch::FileWatch<std::filesystem::path>>(ms_AssetsFolder, [&](const std::filesystem::path& path, const filewatch::Event changeType)
        {
            auto assetPath = ms_AssetsFolder / path;
            if (changeType == filewatch::Event::added)
            {
                Application::Get().SubmitForMainThreadExecution([=]()
                {
                    AssetManager::RegisterAsset(assetPath);
                });
            }
            else if (changeType == filewatch::Event::modified && !ms_PendingReloads[ms_AssetPathUUIDs[assetPath]])
            {
                ms_PendingReloads[ms_AssetPathUUIDs[assetPath]] = true;

                using namespace std::literals;
                std::this_thread::sleep_for(1000ms);

                Application::Get().SubmitForMainThreadExecution([=]()
                {
                    AssetManager::ReloadAsset(ms_AssetPathUUIDs[assetPath]);
                });
            }
            else if (changeType == filewatch::Event::removed)
            {
                UnregisterAsset(assetPath);
            }
        });
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::RegisterAsset(const AssetMetaData& metaData)
    {
        ATOM_ENGINE_ASSERT(metaData.AssetFilepath.extension() == Asset::AssetTypeExtension);

        if (!std::filesystem::exists(metaData.AssetFilepath))
        {
            ATOM_ERROR("Failed registering asset {}. Asset file not found.", metaData.AssetFilepath);
            return;
        }

        ms_Registry[metaData.UUID] = metaData;
        ms_AssetPathUUIDs[metaData.AssetFilepath] = metaData.UUID;
        ms_PendingReloads[metaData.UUID] = false;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool AssetManager::LoadAsset(UUID uuid)
    {
        if (!IsAssetValid(uuid))
        {
            ATOM_ERROR("Failed loading asset with UUID = {}. Asset was not found in registry.", uuid);
            return false;
        }

        if (IsAssetLoaded(uuid))
            return true;

        AssetMetaData& metaData = ms_Registry[uuid];
        Ref<Asset> asset = nullptr;

        switch (metaData.Type)
        {
            case AssetType::Texture2D: asset = AssetSerializer::Deserialize<Texture2D>(metaData.AssetFilepath); break;
            case AssetType::TextureCube: asset = AssetSerializer::Deserialize<TextureCube>(metaData.AssetFilepath); break;
            case AssetType::Material: asset = AssetSerializer::Deserialize<MaterialAsset>(metaData.AssetFilepath); break;
            case AssetType::Mesh: asset = AssetSerializer::Deserialize<Mesh>(metaData.AssetFilepath); break;
        }

        if (!asset)
        {
            ATOM_ERROR("Failed loading asset {}({}). Asset deserialization failed.", metaData.AssetFilepath, uuid);
            return false;
        }

        ms_LoadedAssets[uuid] = asset;
        ATOM_INFO("Successfully loaded asset {}({})", metaData.AssetFilepath, uuid);
        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool AssetManager::ReloadAsset(UUID uuid)
    {
        if (!IsAssetValid(uuid))
            return false;

        AssetMetaData& metaData = ms_Registry[uuid];
        Ref<Asset> asset = nullptr;

        switch (metaData.Type)
        {
            case AssetType::Texture2D: asset = AssetSerializer::Deserialize<Texture2D>(metaData.AssetFilepath); break;
            case AssetType::TextureCube: asset = AssetSerializer::Deserialize<TextureCube>(metaData.AssetFilepath); break;
            case AssetType::Material: asset = AssetSerializer::Deserialize<MaterialAsset>(metaData.AssetFilepath); break;
            case AssetType::Mesh: asset = AssetSerializer::Deserialize<Mesh>(metaData.AssetFilepath); break;
        }

        if (!asset)
        {
            ATOM_ERROR("Failed reloading asset {}({}). Asset deserialization failed.", metaData.AssetFilepath, uuid);
            return false;
        }

        ms_LoadedAssets[uuid] = asset;
        ms_PendingReloads[uuid] = false;
        ATOM_INFO("Asset {}({}) reloaded", metaData.AssetFilepath, uuid);
        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::UnloadAsset(UUID uuid)
    {
        if (!IsAssetValid(uuid) || !IsAssetLoaded(uuid))
            return;

        ms_LoadedAssets.erase(uuid);
        ATOM_INFO("Asset {}({}) unloaded", ms_Registry[uuid].AssetFilepath, uuid);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::UnloadAllAssets()
    {
        ms_LoadedAssets.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const HashMap<UUID, AssetMetaData>& AssetManager::GetRegistry()
    {
        return ms_Registry;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID AssetManager::GetUUIDForAssetPath(const std::filesystem::path& assetPath)
    {
        auto it = ms_AssetPathUUIDs.find(assetPath);

        if (it == ms_AssetPathUUIDs.end())
            return 0;

        return it->second;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::RegisterAllAssets(const std::filesystem::path& assetFolder)
    {
        ATOM_ENGINE_ASSERT(std::filesystem::exists(assetFolder));

        for (auto entry : std::filesystem::directory_iterator(assetFolder))
        {
            if (entry.is_directory())
            {
                RegisterAllAssets(entry);
            }
            else if(entry.path().extension() == Asset::AssetTypeExtension)
            {
                RegisterAsset(entry);
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool AssetManager::IsAssetValid(UUID uuid)
    {
        return ms_Registry.find(uuid) != ms_Registry.end();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool AssetManager::IsAssetLoaded(UUID uuid)
    {
        return ms_LoadedAssets.find(uuid) != ms_LoadedAssets.end();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const AssetMetaData* AssetManager::GetAssetMetaData(UUID uuid)
    {
        auto it = ms_Registry.find(uuid);

        if (it != ms_Registry.end())
            return &it->second;

        return nullptr;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::RegisterAsset(const std::filesystem::path& assetPath)
    {
        if (std::filesystem::is_directory(assetPath))
            return;

        ATOM_ENGINE_ASSERT(assetPath.extension() == Asset::AssetTypeExtension);

        AssetMetaData metaData;
        if (!AssetSerializer::DeserializeMetaData(assetPath, metaData))
        {
            ATOM_ERROR("Failed registering asset {}. Reading asset meta data failed.", assetPath);
            return;
        }

        ms_Registry[metaData.UUID] = metaData;
        ms_AssetPathUUIDs[metaData.AssetFilepath] = metaData.UUID;
        ms_PendingReloads[metaData.UUID] = false;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::UnregisterAsset(const std::filesystem::path& assetPath)
    {
        auto it = ms_AssetPathUUIDs.find(assetPath);

        if (it == ms_AssetPathUUIDs.end())
            return;

        ms_Registry.erase(it->second);
        ms_LoadedAssets.erase(it->second);
        ms_PendingReloads.erase(it->second);
        ms_AssetPathUUIDs.erase(it);
    }
}
