#pragma once

#include <Atom.h>

namespace Atom
{
    class SandboxLayer : public Layer
    {
    public:
        SandboxLayer();
        virtual ~SandboxLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(Timestep ts) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;
    private:
        Ref<CommandBuffer>    m_CommandBuffer = nullptr;
        Ref<GraphicsPipeline> m_DefaultPipeline = nullptr;
        Ref<VertexBuffer>     m_QuadVertexBuffer = nullptr;
        Ref<IndexBuffer>      m_QuadIndexBuffer = nullptr;
        Ref<ConstantBuffer>   m_CameraCB = nullptr;
        EditorCamera          m_Camera;
    };

}