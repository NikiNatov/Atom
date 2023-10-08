#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/MeshAsset.h"
#include "Atom/Renderer/Camera.h"
#include "Atom/Renderer/EditorCamera.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Pipeline.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/LightEnvironment.h"
#include "Atom/Renderer/Renderer.h"

#include "Atom/Renderer/RenderGraph/RenderGraph.h"

namespace Atom
{
    struct CameraConstants
    {
        glm::mat4 ViewMatrix = glm::mat4(1.0f);
        glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 InvViewProjMatrix = glm::mat4(1.0f);
        glm::vec3 CameraPosition = glm::vec3(0.0f);
        f32       CameraExposure = 0.5f;
    };

    struct MeshEntry
    {
        Ref<Mesh>     Mesh;
        u32           SubmeshIndex;
        Ref<Material> Material;
        glm::mat4     Transform;
        u32           BoneTransformIndex = UINT32_MAX;
    };

    struct SceneFrameData
    {
        CameraConstants       CameraConstants;
        Ref<LightEnvironment> Lights;
        Vector<glm::mat4>     BoneTransforms;
        Vector<MeshEntry>     StaticMeshes;
        Vector<MeshEntry>     AnimatedMeshes;
    };

    class SceneRenderer
    {
    public:
        SceneRenderer(bool renderToSwapChain);
        ~SceneRenderer();

        void BeginScene(Camera& camera, const glm::mat4& cameraTransform, const Ref<LightEnvironment>& lightEnvironment);
        void BeginScene(EditorCamera& editorCamera, const Ref<LightEnvironment>& lightEnvironment);
        void SubmitMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialTable>& materialTable);
        void SubmitAnimatedMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, const Ref<MaterialTable>& materialTable, const Ref<Skeleton>& skeleton);
        void SetViewportSize(u32 width, u32 height);
        void Render();

        void OnImGuiRender();

        inline const SceneFrameData& GetSceneFrameData() const { return m_FrameData[Renderer::GetCurrentFrameIndex()]; }
        inline const Texture* GetFinalImage() const { return dynamic_cast<Texture*>(m_RenderGraph.GetFinalOutput()->GetHWResource()); }
        inline u32 GetViewportWidth() const { return m_ViewportWidth; }
        inline u32 GetViewportHeight() const { return m_ViewportHeight; }
    private:
        void PreRender(Ref<CommandBuffer> commandBuffer);
    private:
        static constexpr u32 MaxAnimatedMeshes = 1024;
        static constexpr u32 MaxBonesPerMesh = 100;

        bool                   m_RenderToSwapChain;
        u32                    m_ViewportWidth;
        u32                    m_ViewportHeight;
        Vector<SceneFrameData> m_FrameData;
        RenderGraph            m_RenderGraph;
    };
}