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

    public:
        static void Initialize()
        {
            CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
            Ref<CommandBuffer> copyCommandBuffer = CreateRef<CommandBuffer>(CommandQueueType::Copy, "EditorResourcesCopyCmdBuffer");
            copyCommandBuffer->Begin();

            InfoIcon = LoadResourceTexture(copyCommandBuffer, "resources/icons/info_icon.png", "InfoIcon");
            WarningIcon = LoadResourceTexture(copyCommandBuffer, "resources/icons/warning_icon.png", "WarningIcon");
            ErrorIcon = LoadResourceTexture(copyCommandBuffer, "resources/icons/error_icon.png", "ErrorIcon");

            copyCommandBuffer->End();
            copyQueue->ExecuteCommandList(copyCommandBuffer.get());
            copyQueue->Flush();
        }

        static void Shutdown()
        {
            InfoIcon = nullptr;
            WarningIcon = nullptr;
            ErrorIcon = nullptr;
        }

    private:
        static Ref<Texture2D> LoadResourceTexture(Ref<CommandBuffer> copyCommandBuffer, const String& filepath, const char* name)
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