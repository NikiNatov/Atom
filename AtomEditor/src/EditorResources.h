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
        }

        static void Shutdown()
        {
            InfoIcon = nullptr;
            WarningIcon = nullptr;
            ErrorIcon = nullptr;
            ScenePlayIcon = nullptr;
            SceneStopIcon = nullptr;
            ScenePauseIcon = nullptr;
        }
    };
}