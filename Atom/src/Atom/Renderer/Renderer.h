#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/ShaderLibrary.h"
#include "Atom/Renderer/PipelineLibrary.h"

namespace Atom
{
    class Device;
    class CommandBuffer;
    class GraphicsPipeline;
    class Framebuffer;
    class VertexBuffer;
    class IndexBuffer;
    class Buffer;
    class ConstantBuffer;
    class StructuredBuffer;
    class ReadbackBuffer;
    class Material;
    class Mesh;
    class DescriptorHeap;
    class TextureCube;
    class Texture2D;
    class Texture;
    class EnvironmentMap;

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
        static void BeginRenderPass(Ref<CommandBuffer> commandBuffer, Ref<Framebuffer> framebuffer, bool clear = true);
        static void EndRenderPass(Ref<CommandBuffer> commandBuffer, Ref<Framebuffer> framebuffer);
        static void RenderMesh(Ref<CommandBuffer> commandBuffer, Ref<GraphicsPipeline> pipeline, Ref<Mesh> mesh, u32 submeshIdx, Ref<Material> overrideMaterial, Ref<ConstantBuffer> constantBuffer, Ref<StructuredBuffer> structuredBuffer);
        static void RenderFullscreenQuad(Ref<CommandBuffer> commandBuffer, Ref<GraphicsPipeline> pipeline, Ref<ConstantBuffer> constantBuffer, Ref<Material> material);
        static Ref<TextureCube> CreateEnvironmentMap(Ref<Texture2D> equirectTexture, u32 mapSize);
        static Ref<TextureCube> CreateIrradianceMap(Ref<TextureCube> environmentMap, u32 mapSize);
        static void GenerateMips(Ref<Texture2D> texture);
        static void UploadBufferData(const void* srcData, const Buffer* buffer);
        static void UploadTextureData(const void* srcData, const Texture* texture, u32 mip = 0, u32 slice = 0);
        static Ref<ReadbackBuffer> ReadbackTextureData(const Texture* texture, u32 mip = 0, u32 slice = 0);
        static void EndFrame();

        static const RendererConfig& GetConfig();
        static u32 GetCurrentFrameIndex();
        static u32 GetFramesInFlight();
        static const ShaderLibrary& GetShaderLibrary();
        static const PipelineLibrary& GetPipelineLibrary();
        static Ref<Texture2D> GetBRDF();
        static Ref<Texture2D> GetErrorTexture();
        static Ref<Texture2D> GetBlackTexture();
        static Ref<TextureCube> GetBlackTextureCube();
        static Ref<DescriptorHeap> GetCurrentResourceHeap();
        static Ref<DescriptorHeap> GetCurrentSamplerHeap();
    private:
        inline static RendererConfig              ms_Config;
        inline static Vector<Ref<DescriptorHeap>> ms_ResourceHeaps;
        inline static Vector<Ref<DescriptorHeap>> ms_SamplerHeaps;
        inline static ShaderLibrary               ms_ShaderLibrary;
        inline static PipelineLibrary             ms_PipelineLibrary;
        inline static Ref<VertexBuffer>           ms_FullscreenQuadVB;
        inline static Ref<IndexBuffer>            ms_FullscreenQuadIB;
        inline static Ref<Texture2D>              ms_BRDFTexture;
        inline static Ref<Texture2D>              ms_ErrorTexture;
        inline static Ref<Texture2D>              ms_BlackTexture;
        inline static Ref<TextureCube>            ms_BlackTextureCube;
    };
}