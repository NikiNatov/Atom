#include "atompch.h"
#include "AssetManager.h"

#include "Atom/Asset/AssetSerializer.h"
#include "Atom/Core/Application.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/Mesh.h"
#include "Atom/Scene/Scene.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::Initialize(const std::filesystem::path& assetFolder)
    {
        Shutdown();
        ms_AssetsFolder = std::filesystem::canonical(assetFolder);

        RegisterAllAssets(ms_AssetsFolder);

        ms_FileWatcher = CreateScope<filewatch::FileWatch<std::filesystem::path>>(ms_AssetsFolder, [&](const std::filesystem::path& path, const filewatch::Event changeType)
        {
            auto assetPath = ms_AssetsFolder / path;
            if (changeType == filewatch::Event::added)
            {
                using namespace std::literals;
                std::this_thread::sleep_for(1000ms);

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
    void AssetManager::Shutdown()
    {
        ms_Registry.clear();
        ms_AssetPathUUIDs.clear();
        ms_LoadedAssets.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::RegisterAsset(const AssetMetaData& metaData)
    {
        if (!IsAssetFile(metaData.AssetFilepath))
            return;

        ATOM_ENGINE_ASSERT((metaData.Flags & AssetFlags::Serialized) != AssetFlags::None);

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
    void AssetManager::RegisterAsset(const Ref<Asset>& asset)
    {
        const AssetMetaData& metaData = asset->GetMetaData();

        if (asset->GetAssetFlag(AssetFlags::Serialized))
        {
            if (!std::filesystem::exists(metaData.AssetFilepath))
            {
                ATOM_ERROR("Failed registering asset {}. Asset file not found.", metaData.AssetFilepath);
                return;
            }

            ms_AssetPathUUIDs[metaData.AssetFilepath] = metaData.UUID;
            ms_PendingReloads[metaData.UUID] = false;
        }

        ms_Registry[metaData.UUID] = metaData;
        ms_LoadedAssets[metaData.UUID] = asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::RegisterAsset(const std::filesystem::path& assetPath)
    {
        if (!IsAssetFile(assetPath))
            return;

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
    void AssetManager::UnregisterAsset(UUID uuid)
    {
        if (!IsAssetValid(uuid))
            return;
        
        ms_AssetPathUUIDs.erase(ms_Registry[uuid].AssetFilepath);
        ms_Registry.erase(uuid);
        ms_LoadedAssets.erase(uuid);
        ms_PendingReloads.erase(uuid);

        ATOM_INFO("Asset {} unregistered", uuid);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManager::UnregisterAsset(const std::filesystem::path& assetPath)
    {
        auto it = ms_AssetPathUUIDs.find(assetPath);

        if (it == ms_AssetPathUUIDs.end())
            return;

        UUID uuid = it->second;
        ms_Registry.erase(uuid);
        ms_LoadedAssets.erase(uuid);
        ms_PendingReloads.erase(uuid);
        ms_AssetPathUUIDs.erase(it);

        ATOM_INFO("Asset {} unregistered", uuid);
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
            case AssetType::Material: asset = AssetSerializer::Deserialize<Material>(metaData.AssetFilepath); break;
            case AssetType::Mesh: asset = AssetSerializer::Deserialize<Mesh>(metaData.AssetFilepath); break;
            case AssetType::Scene: asset = AssetSerializer::Deserialize<Scene>(metaData.AssetFilepath); break;
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

        if (!IsAssetLoaded(uuid))
            return LoadAsset(uuid);

        AssetMetaData& metaData = ms_Registry[uuid];
        bool result = false;

        switch (metaData.Type)
        {
            case AssetType::Texture2D:
            {
                Ref<Texture2D> textureAsset = AssetSerializer::Deserialize<Texture2D>(metaData.AssetFilepath);
                result = textureAsset != nullptr;

                if(result)
                    *std::dynamic_pointer_cast<Texture2D>(ms_LoadedAssets[uuid]) = std::move(*textureAsset);

                break;
            }
            case AssetType::TextureCube:
            {
                Ref<TextureCube> textureAsset = AssetSerializer::Deserialize<TextureCube>(metaData.AssetFilepath); 
                result = textureAsset != nullptr;

                if (result)
                    *std::dynamic_pointer_cast<TextureCube>(ms_LoadedAssets[uuid]) = std::move(*textureAsset);

                break;
            }
            case AssetType::Material:
            {
                Ref<Material> materialAsset = AssetSerializer::Deserialize<Material>(metaData.AssetFilepath);
                result = materialAsset != nullptr;

                if (result)
                    *std::dynamic_pointer_cast<Material>(ms_LoadedAssets[uuid]) = std::move(*materialAsset);

                break;
            }
            case AssetType::Mesh:
            {
                Ref<Mesh> meshAsset = AssetSerializer::Deserialize<Mesh>(metaData.AssetFilepath);
                result = meshAsset != nullptr;

                if (result)
                    *std::dynamic_pointer_cast<Mesh>(ms_LoadedAssets[uuid]) = std::move(*meshAsset);

                break;
            }
            case AssetType::Scene:
            {
                Ref<Scene> sceneAsset = AssetSerializer::Deserialize<Scene>(metaData.AssetFilepath);
                result = sceneAsset != nullptr;

                if (result)
                    *std::dynamic_pointer_cast<Scene>(ms_LoadedAssets[uuid]) = std::move(*sceneAsset);

                break;
            }
        }

        if (!result)
        {
            ATOM_ERROR("Failed reloading asset {}({}). Asset deserialization failed.", metaData.AssetFilepath, uuid);
            return false;
        }

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
    void AssetManager::UnloadUnusedAssets()
    {
        Vector<Ref<Asset>> assetsToUnload;
        assetsToUnload.reserve(ms_LoadedAssets.size());

        for (auto& [uuid, asset] : ms_LoadedAssets)
        {
            if (asset.use_count() == 1)
                assetsToUnload.push_back(asset);
        }

        for (auto& asset : assetsToUnload)
        {
            if (asset->GetAssetFlag(AssetFlags::Serialized))
                UnloadAsset(asset->GetUUID());
            else
                UnregisterAsset(asset->GetUUID());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const HashMap<UUID, AssetMetaData>& AssetManager::GetRegistry()
    {
        return ms_Registry;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID AssetManager::GetUUIDForAssetPath(const std::filesystem::path& assetPath)
    {
        auto it = ms_AssetPathUUIDs.find(GetAssetFullPath(assetPath));

        if (it == ms_AssetPathUUIDs.end())
            return 0;

        return it->second;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    std::filesystem::path AssetManager::GetAssetFullPath(const std::filesystem::path& assetPath)
    {
        return ms_AssetsFolder / assetPath;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const std::filesystem::path& AssetManager::GetAssetsFolder()
    {
        return ms_AssetsFolder;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool AssetManager::IsAssetFile(const std::filesystem::path& filepath)
    {
        for (u32 i = 0; i < (u32)AssetType::NumTypes; i++)
            if (filepath.extension() == Asset::AssetFileExtensions[i])
                return true;

        return false;
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
            else if(IsAssetFile(entry.path()))
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
    u32 AssetManager::GetAssetRefCount(UUID uuid)
    {
        if (!IsAssetValid(uuid) || !IsAssetLoaded(uuid))
            return 0;

        return ms_LoadedAssets.at(uuid).use_count() - 1;
    }
}
