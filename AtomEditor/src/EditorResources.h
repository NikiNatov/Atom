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
            CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
            Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
            copyCommandBuffer->Begin();

            InfoIcon = LoadResourceTexture(copyCommandBuffer, "resources/icons/info_icon.png", "InfoIcon");
            WarningIcon = LoadResourceTexture(copyCommandBuffer, "resources/icons/warning_icon.png", "WarningIcon");
            ErrorIcon = LoadResourceTexture(copyCommandBuffer, "resources/icons/error_icon.png", "ErrorIcon");
            ScenePlayIcon = LoadResourceTexture(copyCommandBuffer, "resources/icons/scene_play_icon.png", "ScenePlayIcon");
            SceneStopIcon = LoadResourceTexture(copyCommandBuffer, "resources/icons/scene_stop_icon.png", "SceneStopIcon");
            ScenePauseIcon = LoadResourceTexture(copyCommandBuffer, "resources/icons/scene_pause_icon.png", "ScenePauseIcon");

            copyCommandBuffer->End();
            copyQueue->ExecuteCommandList(copyCommandBuffer);
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

    private:
        static Ref<Texture2D> LoadResourceTexture(Ref<CommandBuffer> copyCommandBuffer, const std::filesystem::path& filepath, const char* name)
        {
            Image2D image(filepath);

            TextureDescription desc;
            desc.Format = TextureFormat::RGBA8;
            desc.Width = image.GetWidth();
            desc.Height = image.GetHeight();

            Ref<Texture2D> texture = CreateRef<Texture2D>(desc, name);
            copyCommandBuffer->UploadTextureData(image.GetPixelData().data(), texture.get());

            return texture;
        }
    };
}