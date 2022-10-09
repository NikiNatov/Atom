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
        void RunComputeShaderTest();
    private:
        Ref<GraphicsPipeline> m_GeometryPipeline = nullptr;
        Ref<ConstantBuffer>   m_CameraCB = nullptr;
        Ref<Mesh>             m_TestMesh = nullptr;
        Ref<Texture2D>        m_ComputeShaderTestTexture = nullptr;
        EditorCamera          m_Camera;
        glm::vec2             m_ViewportSize = { 0.0f, 0.0f };
    };

}