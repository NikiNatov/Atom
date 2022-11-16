#pragma once

#include <Atom.h>

namespace Atom
{
    class SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        ~SceneHierarchyPanel() = default;

        void OnImGuiRender();

        void SetScene(const Ref<Scene>& scene);
        inline Entity GetSelectedEntity() const { return m_SelectedEntity; }
    private:
        void DrawEntityNode(Entity entity);
    private:
        Ref<Scene> m_Scene;
        Entity     m_SelectedEntity = {};
    };
}