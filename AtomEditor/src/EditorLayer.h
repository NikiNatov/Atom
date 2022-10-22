#pragma once

#include <Atom.h>

namespace Atom
{
    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(Timestep ts) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;
    private:
        Ref<GraphicsPipeline>                         m_GeometryPipeline = nullptr;
        Ref<GraphicsPipeline>                         m_SkyBoxPipeline = nullptr;
        Ref<GraphicsPipeline>                         m_CompositePipeline = nullptr;
        Ref<ConstantBuffer>                           m_CameraCB = nullptr;
        Ref<StructuredBuffer>                         m_LightsSB = nullptr;
        Ref<Mesh>                                     m_TestMesh = nullptr;
        Ref<Material>                                 m_SkyBoxMaterial = nullptr;
        Ref<Material>                                 m_CompositeMaterial = nullptr;
        std::pair<Ref<TextureCube>, Ref<TextureCube>> m_EnvironmentMap;
        EditorCamera                                  m_Camera;
        glm::vec2                                     m_ViewportSize = { 0.0f, 0.0f };
        bool                                          m_NeedsResize = false;
    };

}