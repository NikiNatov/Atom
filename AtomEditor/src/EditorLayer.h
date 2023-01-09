#pragma once

#include <Atom.h>

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/EntityInspectorPanel.h"
#include "Panels/AssetPanel.h"
#include "Panels/MaterialEditorPanel.h"
#include "Dialogs/ImportDialog.h"
#include "Dialogs/NewProjectDialog.h"
#include "Dialogs/NewMaterialDialog.h"

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

        void NewProject(const String& projectName, const String& startSceneName, const std::filesystem::path& projectLocation);
        void OpenProject();
        void OpenProject(const std::filesystem::path& filepath);
        void SaveProject();

        void SaveScene();
        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& assetPath);
        void PlayScene();
        void StopScene();
        void DuplicateEntity();

        inline const SceneHierarchyPanel& GetSceneHierarchyPanel() const { return m_SceneHierarchyPanel; }
        inline const EntityInspectorPanel& GetEntityInspectorPanel() const { return m_EntityInspectorPanel; }

        static EditorLayer& Get() { return *ms_Instance; }
    private:
        bool OnKeyPressed(KeyPressedEvent& e);

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
        AssetPanel           m_AssetPanel;
        MaterialEditorPanel  m_MaterialEditorPanel;
        TextureImportDialog  m_TextureImportDialog;
        MeshImportDialog     m_MeshImportDialog;
        NewProjectDialog     m_NewProjectDialog;
        NewMaterialDialog    m_NewMaterialDialog;
    private:
        inline static EditorLayer* ms_Instance = nullptr;
    };

}