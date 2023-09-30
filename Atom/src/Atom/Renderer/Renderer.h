#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/ShaderLibrary.h"
#include "Atom/Renderer/PipelineLibrary.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/TextureSampler.h"

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
        static void RenderMesh(Ref<CommandBuffer> commandBuffer, Ref<Mesh> mesh, u32 submeshIdx, Ref<Material> overrideMaterial);
        static void RenderFullscreenQuad(Ref<CommandBuffer> commandBuffer, Texture* texture);
        static Ref<Texture> CreateEnvironmentMap(Ref<Texture> equirectTexture, u32 mapSize, const char* debugName);
        static Ref<Texture> CreateIrradianceMap(Ref<Texture> environmentMap, u32 mapSize, const char* debugName);
        static Ref<Texture> CreateBRDFTexture();
        static void GenerateMips(Ref<Texture> texture);
        static void UploadBufferData(const void* srcData, const Buffer* buffer);
        static void UploadTextureData(const void* srcData, Ref<Texture> texture, u32 mip = 0, u32 slice = 0);
        static Ref<ReadbackBuffer> ReadbackTextureData(Ref<Texture> texture, u32 mip = 0, u32 slice = 0);
        static void EndFrame();

        static const RendererConfig& GetConfig();
        static u32 GetCurrentFrameIndex();
        static u32 GetFramesInFlight();
        static ShaderLibrary& GetShaderLibrary();
        static PipelineLibrary& GetPipelineLibrary();
        static Ref<Texture> GetBRDF();
        static Ref<Texture> GetErrorTexture();
        static Ref<Texture> GetBlackTexture();
        static Ref<Texture> GetBlackTextureCube();
        static Ref<Material> GetDefaultMaterial();
        static Ref<Material> GetDefaultMaterialAnimated();
        static Ref<Material> GetErrorMaterial();
        static Ref<Material> GetErrorMaterialAnimated();
        static Ref<TextureSampler> GetSampler(TextureFilter filter, TextureWrap wrap);
    private:
        inline static RendererConfig              ms_Config;
        inline static ShaderLibrary               ms_ShaderLibrary;
        inline static PipelineLibrary             ms_PipelineLibrary;
        inline static Ref<VertexBuffer>           ms_FullscreenQuadVB;
        inline static Ref<IndexBuffer>            ms_FullscreenQuadIB;
        inline static Ref<Texture>                ms_BRDFTexture;
        inline static Ref<Texture>                ms_ErrorTexture;
        inline static Ref<Texture>                ms_BlackTexture;
        inline static Ref<Texture>                ms_BlackTextureCube;
        inline static Ref<Material>               ms_DefaultMaterial;
        inline static Ref<Material>               ms_DefaultMaterialAnimated;
        inline static Ref<Material>               ms_ErrorMaterial;
        inline static Ref<Material>               ms_ErrorMaterialAnimated;
        inline static Vector<Ref<TextureSampler>> ms_Samplers;
    };
}