#pragma once

#include "Atom/Core/Core.h"

#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/TextureSampler.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/Camera.h"
#include "Atom/Renderer/EditorCamera.h"

#include "Atom/Renderer/RenderGraph/RenderGraph.h"
#include "Atom/Renderer/RenderGraph/ResourceScheduler.h"

#include "Atom/Asset/MeshAsset.h"
#include "Atom/Asset/MaterialAsset.h"

#include <autogen/cpp/MeshDrawParams.h>

namespace Atom
{
    enum class LightType
    {
        DirLight,
        PointLight,
        SpotLight
    };

    struct Light
    {
        glm::vec4 Position;
        glm::vec4 Direction;
        glm::vec4 Color;
        f32       Intensity;
        f32       ConeAngle;
        glm::vec3 AttenuationFactors;
        LightType Type;
    };

    struct MeshEntry
    {
        Ref<Mesh>           Mesh;
        u32                 SubmeshIndex;
        Ref<Material>       Material;
        SIG::MeshDrawParams DrawParams;
    };

    struct ShadowCascade
    {
        glm::mat4 LightViewProjMatrix = glm::mat4(1.0f);
        f32       SplitDistance = 0.0f;
    };

    struct RendererSpecification
    {
        bool RenderToSwapChain = false;
        u32  NumShadowCascades = 4;
    };

    class Renderer
    {
    public:
        struct RendererFrameData
        {
            glm::mat4             ViewMatrix = glm::mat4(1.0f);
            glm::mat4             ProjectionMatrix = glm::mat4(1.0f);
            glm::mat4             InvViewProjMatrix = glm::mat4(1.0f);
            glm::vec3             CameraPosition = glm::vec3(0.0f);
            f32                   CameraExposure = 0.5f;
            u32                   ViewportWidth = 1;
            u32                   ViewportHeight = 1;
            Vector<ShadowCascade> ShadowCascades;
            Vector<Light>         Lights;
            Vector<MeshEntry>     StaticMeshes;
            Vector<MeshEntry>     AnimatedMeshes;
            Vector<glm::mat4>     BoneTransforms;

            // Per frame GPU resources
            Ref<Texture>          EnvironmentMaps[g_FramesInFlight];
            Ref<Texture>          IrradianceMaps[g_FramesInFlight];
            Ref<StructuredBuffer> ShadowCascadeGPUBuffers[g_FramesInFlight];
            Ref<StructuredBuffer> LightsGPUBuffers[g_FramesInFlight];
            Ref<StructuredBuffer> BoneTransformsGPUBuffers[g_FramesInFlight];
        };
    public:
        Renderer(const RendererSpecification& spec = RendererSpecification());

        void BeginScene(const Camera& camera, const glm::mat4& cameraTransform, const Ref<Texture>& environmentMap, const Ref<Texture>& irradianceMap);
        void BeginScene(const EditorCamera& editorCamera, const Ref<Texture>& environmentMap, const Ref<Texture>& irradianceMap);
        void SubmitDirectionalLight(const glm::vec3& color, const glm::vec3& direction, f32 intensity);
        void SubmitPointLight(const glm::vec3& color, const glm::vec3& position, f32 intensity, const glm::vec3& attenuationFactors);
        void SubmitSpotLight(const glm::vec3& color, const glm::vec3& position, const glm::vec3& direction, f32 intensity, f32 coneAngle, const glm::vec3& attenuationFactors);
        void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialTable>& materialTable);
        void SubmitAnimatedMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialTable>& materialTable, const Ref<Skeleton>& skeleton);
        void SetViewportSize(u32 width, u32 height);
        void Render();

        void OnImGuiRender();

        const Texture* GetFinalImage() const;
        inline const RendererSpecification& GetSpecification() const { return m_Specification; }
    public:
        static Ref<Texture> CreateEnvironmentMap(Ref<Texture> equirectTexture, u32 mapSize, const char* debugName);
        static Ref<Texture> CreateIrradianceMap(Ref<Texture> environmentMap, u32 mapSize, const char* debugName);
        static void GenerateMips(Ref<Texture> texture);
        static void UploadBufferData(Ref<Buffer> buffer, const void* srcData);
        static void UploadTextureData(Ref<Texture> texture, const void* srcData, u32 mip = 0, u32 slice = 0);
        static Ref<ReadbackBuffer> ReadbackTextureData(Ref<Texture> texture, u32 mip = 0, u32 slice = 0);
        static Ref<TextureSampler> GetSampler(TextureFilter filter, TextureWrap wrap);
    private:
        void BuildRenderPasses();
        void PreRender();
        void RecordCommandBuffers();
        void ExecuteCommandBuffers();
    private:
        static constexpr u32 MaxAnimatedMeshes = 1024;
        static constexpr u32 MaxBonesPerMesh = 100;
        static constexpr u32 MaxShadowCascades = 4;
        static constexpr f32 ShadowCascadeSplitDistanceFactors[MaxShadowCascades] = { 0.02f, 0.06f, 0.15f, 0.20f };

        RendererSpecification m_Specification;

        ResourceScheduler     m_ResourceSchedulers[g_FramesInFlight];
        RenderGraph           m_RenderGraph;
        RendererFrameData     m_FrameData;
        
    };
}