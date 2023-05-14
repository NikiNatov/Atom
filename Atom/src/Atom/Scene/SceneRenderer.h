#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Camera.h"
#include "Atom/Renderer/EditorCamera.h"
#include "Atom/Renderer/Mesh.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/Framebuffer.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/LightEnvironment.h"

namespace Atom
{
    class SceneRenderer
    {
    public:
        SceneRenderer() = default;
        ~SceneRenderer();

        void Initialize();
        void BeginScene(Camera& camera, const glm::mat4& cameraTransform, const Ref<LightEnvironment>& lightEnvironment);
        void BeginScene(EditorCamera& editorCamera, const Ref<LightEnvironment>& lightEnvironment);
        void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialTable>& materialTable);
        void SubmitAnimatedMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialTable>& materialTable, const Ref<Skeleton>& skeleton);
        void OnViewportResize(u32 width, u32 height);
        void Flush();

        inline Ref<Framebuffer> GetGeometryPassTarget() const { return m_GeometryPipeline->GetFramebuffer(); }
        inline Ref<Framebuffer> GetFinalPassTarget() const { return m_CompositePipeline->GetFramebuffer(); }

        inline Ref<RenderTexture2D> GetFinalImage() const { return GetFinalPassTarget()->GetColorAttachment(AttachmentPoint::Color0); }
    private:
        void PreRender(Ref<CommandBuffer> commandBuffer);
        void GeometryPass(Ref<CommandBuffer> commandBuffer);
        void CompositePass(Ref<CommandBuffer> commandBuffer);
    private:
        struct DrawCommand
        {
            Ref<Mesh>     Mesh;
            u32           SubmeshIndex;
            Ref<Material> Material;
            glm::mat4     Transform;
        };

        struct AnimatedDrawCommand
        {
            Ref<Mesh>     Mesh;
            u32           SubmeshIndex;
            Ref<Material> Material;
            glm::mat4     Transform;
            u32           BoneTransformIndex;
        };

        struct FrameConstants
        {
            glm::mat4 ViewMatrix = glm::mat4(1.0f);
            glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
            glm::mat4 InvViewProjMatrix = glm::mat4(1.0f);
            glm::vec3 CameraPosition = glm::vec3(0.0f);
            f32 CameraExposure = 0.5f;
            u32 NumLights = 0;
            f32 p[11]{ 0 };
        };

        static constexpr u32 MaxAnimatedMeshes = 1024;
        static constexpr u32 MaxBonesPerMesh = 100;

        Vector<DrawCommand>         m_DrawList;
        Vector<AnimatedDrawCommand> m_DrawListAnimated;

        Vector<FrameConstants>        m_FrameConstants;
        Vector<Ref<ConstantBuffer>>   m_FrameCBs;
        Vector<Vector<glm::mat4>>     m_BoneTransforms;
        Vector<Ref<StructuredBuffer>> m_BoneTransformsSBs;
        Vector<Ref<LightEnvironment>> m_Lights;
        Vector<Ref<StructuredBuffer>> m_LightsSBs;

        DescriptorAllocation m_FrameResourceTable;
        DescriptorAllocation m_FrameSamplerTable;

        Ref<GraphicsPipeline> m_GeometryPipeline = nullptr;
        Ref<GraphicsPipeline> m_AnimatedGeometryPipeline = nullptr;
        Ref<GraphicsPipeline> m_CompositePipeline = nullptr;
        Ref<GraphicsPipeline> m_SkyBoxPipeline = nullptr;
        Ref<GraphicsPipeline> m_FullScreenQuadPipeline = nullptr;
    };
}