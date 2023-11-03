#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/ShaderLibrary.h"
#include "Atom/Renderer/PipelineLibrary.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/TextureSampler.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/CommandBuffer.h"

namespace Atom
{
    class Mesh;

    struct RendererConfig
    {
        u32 FramesInFlight = 3;
        u32 MaxDescriptorsPerHeap = 2048;
    };

    class Renderer
    {
    public:
        static void Initialize(const RendererConfig& config = RendererConfig());
        static void Shutdown();
        static void BeginFrame();
        static void RenderMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, u32 submeshIdx, Ref<Material> overrideMaterial);
        static void RenderFullscreenQuad(Ref<CommandBuffer> commandBuffer, Texture* texture);
        static Ref<Texture> CreateEnvironmentMap(Ref<Texture> equirectTexture, u32 mapSize, const char* debugName);
        static Ref<Texture> CreateIrradianceMap(Ref<Texture> environmentMap, u32 mapSize, const char* debugName);
        static Ref<Texture> CreateBRDFTexture();
        static void GenerateMips(Ref<Texture> texture);
        static void UploadBufferData(Ref<Buffer> buffer, const void* srcData);
        static void UploadTextureData(Ref<Texture> texture, const void* srcData, u32 mip = 0, u32 slice = 0);
        static Ref<ReadbackBuffer> ReadbackTextureData(Ref<Texture> texture, u32 mip = 0, u32 slice = 0);
        static void EndFrame();

        static const RendererConfig& GetConfig();
        static u32 GetCurrentFrameIndex();
        static u32 GetFramesInFlight();
        static ShaderLibrary& GetShaderLibrary();
        static PipelineLibrary& GetPipelineLibrary();
        static Ref<Texture> GetBRDF();
        static Ref<TextureSampler> GetSampler(TextureFilter filter, TextureWrap wrap);
    private:
        inline static RendererConfig              ms_Config;
        inline static ShaderLibrary               ms_ShaderLibrary;
        inline static PipelineLibrary             ms_PipelineLibrary;
        inline static Ref<Texture>                ms_BRDFTexture;
    };
}