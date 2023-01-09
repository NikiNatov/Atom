#pragma once

#include <Atom.h>

namespace Atom
{
    class RuntimeLayer : public Layer
    {
    public:
        RuntimeLayer();
        virtual ~RuntimeLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate(Timestep ts) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent(Event& event) override;

        void OpenProject(const std::filesystem::path& filepath);
    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnWindowResized(WindowResizedEvent& e);
    private:
        Ref<Scene>            m_ActiveScene = nullptr;
        Ref<SceneRenderer>    m_SceneRenderer = nullptr;
        Ref<GraphicsPipeline> m_SwapChainPipeline = nullptr;
        Ref<Material>         m_SwapChainMaterial = nullptr;
    };

}