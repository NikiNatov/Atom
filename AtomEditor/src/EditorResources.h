#pragma once

#include <Atom.h>

namespace Atom
{
    class EditorResources
    {
    public:
        inline static Ref<Texture> InfoIcon = nullptr;
        inline static Ref<Texture> WarningIcon = nullptr;
        inline static Ref<Texture> ErrorIcon = nullptr;
        inline static Ref<Texture> ScenePlayIcon = nullptr;
        inline static Ref<Texture> SceneStopIcon = nullptr;
        inline static Ref<Texture> ScenePauseIcon = nullptr;
        inline static Ref<Texture> FolderIcon = nullptr;
        inline static Ref<Texture> Texture2DAssetIcon = nullptr;
        inline static Ref<Texture> TextureCubeAssetIcon = nullptr;
        inline static Ref<Texture> MeshAssetIcon = nullptr;
        inline static Ref<Texture> MaterialAssetIcon = nullptr;
        inline static Ref<Texture> SceneAssetIcon = nullptr;
        inline static Ref<Texture> AnimationAssetIcon = nullptr;
        inline static Ref<Texture> SkeletonAssetIcon = nullptr;
        inline static Ref<Texture> AnimationControllerAssetIcon = nullptr;

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
            AnimationControllerAssetIcon = ContentTools::ImportTexture("resources/icons/animation_controller_asset_icon.png", importSettings);
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
            AnimationControllerAssetIcon = nullptr;
        }
    };
}