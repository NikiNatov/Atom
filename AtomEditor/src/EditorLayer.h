#pragma once

#include <Atom.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/EntityInspectorPanel.h"
#include "Dialogs/ImportDialog.h"

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

        inline const SceneHierarchyPanel& GetSceneHierarchyPanel() const { return m_SceneHierarchyPanel; }
        inline const EntityInspectorPanel& GetEntityInspectorPanel() const { return m_EntityInspectorPanel; }

        static EditorLayer& Get() { return *ms_Instance; }
    private:
        bool OnKeyPressed(KeyPressedEvent& e);

        void SaveScene();
        void NewScene();
        void OpenScene();
        void PlayScene();
        void StopScene();
        void DuplicateEntity();
    private:
        Ref<Scene>           m_ActiveScene = nullptr;
        Ref<Scene>           m_EditorScene = nullptr;
        Ref<SceneRenderer>   m_Renderer = nullptr;
        glm::vec2            m_ViewportSize = { 0.0f, 0.0f };
        bool                 m_NeedsResize = false;
        s32                  m_GuizmoOperation = -1;
        bool                 m_GuizmoSnap = false;
        SceneHierarchyPanel  m_SceneHierarchyPanel;
        EntityInspectorPanel m_EntityInspectorPanel;
        TextureImportDialog  m_TextureImportDialog;
        MeshImportDialog     m_MeshImportDialog;
    private:
        inline static EditorLayer* ms_Instance = nullptr;
    };

}