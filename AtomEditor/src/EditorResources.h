#pragma once

#include <Atom.h>

namespace Atom
{
    class EditorResources
    {
    public:
        inline static Ref<Texture2D> InfoIcon = nullptr;
        inline static Ref<Texture2D> WarningIcon = nullptr;
        inline static Ref<Texture2D> ErrorIcon = nullptr;
        inline static Ref<Texture2D> ScenePlayIcon = nullptr;
        inline static Ref<Texture2D> SceneStopIcon = nullptr;
        inline static Ref<Texture2D> ScenePauseIcon = nullptr;
        inline static Ref<Texture2D> FolderIcon = nullptr;
        inline static Ref<Texture2D> Texture2DAssetIcon = nullptr;
        inline static Ref<Texture2D> TextureCubeAssetIcon = nullptr;
        inline static Ref<Texture2D> MeshAssetIcon = nullptr;
        inline static Ref<Texture2D> MaterialAssetIcon = nullptr;
        inline static Ref<Texture2D> SceneAssetIcon = nullptr;
        inline static Ref<Texture2D> AnimationAssetIcon = nullptr;
        inline static Ref<Texture2D> SkeletonAssetIcon = nullptr;

    public:
        static void Initialize()
        {

            TextureImportSettings importSettings = TextureImportSettings();
            InfoIcon = ContentTools::ImportTexture("resources/icons/info_icon.png", importSettings);
            WarningIcon = ContentTools::ImportTexture("resources/icons/warning_icon.png", importSettings);
            ErrorIcon = ContentTools::ImportTexture("resources/icons/error_icon.png", importSettings);
            ScenePlayIcon = ContentTools::ImportTexture("resources/icons/scene_play_icon.png", importSettings);
            SceneStopIcon = ContentTools::ImportTexture("resources/icons/scene_stop_icon.png", importSettings);
            ScenePauseIcon = ContentTools::ImportTexture("resources/icons/scene_pause_icon.png", importSettings);
            FolderIcon = ContentTools::ImportTexture("resources/icons/folder_icon.png", importSettings);
            Texture2DAssetIcon = ContentTools::ImportTexture("resources/icons/texture2d_asset_icon.png", importSettings);
            TextureCubeAssetIcon = ContentTools::ImportTexture("resources/icons/texture_cube_asset_icon.png", importSettings);
            MeshAssetIcon = ContentTools::ImportTexture("resources/icons/mesh_asset_icon.png", importSettings);
            MaterialAssetIcon = ContentTools::ImportTexture("resources/icons/material_asset_icon.png", importSettings);
            SceneAssetIcon = ContentTools::ImportTexture("resources/icons/scene_asset_icon.png", importSettings);
            AnimationAssetIcon = ContentTools::ImportTexture("resources/icons/animation_asset_icon.png", importSettings);
            SkeletonAssetIcon = ContentTools::ImportTexture("resources/icons/skeleton_asset_icon.png", importSettings);
        }

        static void Shutdown()
        {
            InfoIcon = nullptr;
            WarningIcon = nullptr;
            ErrorIcon = nullptr;
            ScenePlayIcon = nullptr;
            SceneStopIcon = nullptr;
            ScenePauseIcon = nullptr;
            FolderIcon = nullptr;
            Texture2DAssetIcon = nullptr;
            TextureCubeAssetIcon = nullptr;
            MeshAssetIcon = nullptr;
            MaterialAssetIcon = nullptr;
            SceneAssetIcon = nullptr;
            AnimationAssetIcon = nullptr;
            SkeletonAssetIcon = nullptr;
        }
    };
}