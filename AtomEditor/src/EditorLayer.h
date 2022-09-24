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
        Ref<CommandBuffer>    m_CommandBuffer = nullptr;
        Ref<GraphicsPipeline> m_DefaultPipeline = nullptr;
        Ref<ConstantBuffer>   m_CameraCB = nullptr;
        Ref<Mesh>             m_TestMesh = nullptr;
        EditorCamera          m_Camera;
        glm::vec2             m_ViewportSize = { 0.0f, 0.0f };
    };

}