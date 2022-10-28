#pragma once

#include <Atom.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/EntityInspectorPanel.h"

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
        Ref<Scene>           m_Scene = nullptr;
        Ref<SceneRenderer>   m_Renderer = nullptr;
        glm::vec2            m_ViewportSize = { 0.0f, 0.0f };
        bool                 m_NeedsResize = false;
        SceneHierarchyPanel  m_SceneHierarchyPanel;
        EntityInspectorPanel m_EntityInspectorPanel;
    };

}