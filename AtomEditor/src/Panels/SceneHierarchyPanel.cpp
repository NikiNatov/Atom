#include "SceneHierarchyPanel.h"

#include <imgui/imgui.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");

        m_Scene->m_Registry.each([&](auto entityID)
        {
            Entity entity(entityID, m_Scene.get());

			auto& name = entity.GetComponent<TagComponent>().Tag;

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | (m_SelectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0);

			bool isOpen = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, name.c_str());

			if (ImGui::IsItemClicked())
				m_SelectedEntity = entity;

			ImGui::PushID((void*)(uint64_t)(uint32_t)entity);

			bool removeEntity = false;
			if (ImGui::BeginPopupContextItem())
			{
				m_SelectedEntity = entity;

				if (ImGui::MenuItem("Remove Entity"))
				{
					removeEntity = true;
				}

				ImGui::EndPopup();
			}
			ImGui::PopID();

			if (isOpen)
			{
				ImGui::TreePop();
			}

			if (removeEntity)
			{
				if (m_SelectedEntity == entity)
					m_SelectedEntity = {};

				m_Scene->DeleteEntity(entity);
			}
        });

        if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
            m_SelectedEntity = {};

        ImGui::End();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneHierarchyPanel::SetScene(const Ref<Scene>& scene)
    {
        m_Scene = scene;
        m_SelectedEntity = {};
    }
}
