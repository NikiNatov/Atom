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
        void BeginScene(Camera& camera, const glm::mat4& cameraTransform, const LightEnvironment& lightEnvironment);
        void BeginScene(EditorCamera& editorCamera, const LightEnvironment& lightEnvironment);
        void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Vector<Ref<Material>>& materials);
        void OnViewportResize(u32 width, u32 height);
        void PreRender();
        void Flush();

        inline Ref<Framebuffer> GetGeometryPassTarget() const { return m_GeometryPipeline->GetFramebuffer(); }
        inline Ref<Framebuffer> GetFinalPassTarget() const { return m_CompositePipeline->GetFramebuffer(); }

        inline Ref<RenderTexture2D> GetFinalImage() const { return GetFinalPassTarget()->GetColorAttachment(AttachmentPoint::Color0); }
    private:
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

        struct TransformCB
        {
            glm::mat4 ViewMatrix = glm::mat4(1.0f);
            glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
            glm::vec3 CameraPosition = glm::vec3(0.0f);
            f32 p[29]{ 0 };
        };

        Ref<TextureCube>      m_EnvironmentMap = nullptr;
        Ref<TextureCube>      m_IrradianceMap = nullptr;

        Vector<DrawCommand>   m_DrawList;
        TransformCB           m_TransformData;
        Ref<ConstantBuffer>   m_TransformCB = nullptr;
        Ref<StructuredBuffer> m_LightsSB = nullptr;

        Ref<GraphicsPipeline> m_GeometryPipeline = nullptr;
        Ref<GraphicsPipeline> m_CompositePipeline = nullptr;
        Ref<Material>         m_CompositeMaterial = nullptr;
        Ref<GraphicsPipeline> m_SkyBoxPipeline = nullptr;
        Ref<Material>         m_SkyBoxMaterial = nullptr;
        Ref<GraphicsPipeline> m_FullScreenQuadPipeline = nullptr;
        Ref<Material>         m_FullScreenQuadMaterial = nullptr;


    };
}