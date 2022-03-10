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
        Ref<CommandBuffer> m_CommandBuffer;
        Ref<Texture2D> m_Texture;
    };

}