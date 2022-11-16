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

			if (!entity.GetComponent<SceneHierarchyComponent>().Parent)
			{
				DrawEntityNode(entity);
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

	// -----------------------------------------------------------------------------------------------------------------------------
	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | (m_SelectedEntity == entity ? ImGuiTreeNodeFlags_Selected : 0);

		bool isOpen = ImGui::TreeNodeEx((void*)(u64)(u32)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
			m_SelectedEntity = entity;

		ImGui::PushID((void*)(u64)(u32)entity);

		bool removeEntity = false;
		if (ImGui::BeginPopupContextItem())
		{
			m_SelectedEntity = entity;

			if (ImGui::BeginMenu("New child"))
			{
				if (ImGui::MenuItem("Empty entity"))
				{
					Entity newEntity = m_Scene->CreateEntity();
					newEntity.AddComponent<MeshComponent>(CreateRef<Mesh>("assets/meshes/sphere.gltf"));
					entity.AddChild(newEntity);
				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Remove Entity"))
			{
				removeEntity = true;
			}

			ImGui::EndPopup();
		}
		ImGui::PopID();

		if (isOpen)
		{
			Entity currentChild = entity.GetComponent<SceneHierarchyComponent>().FirstChild;

			while (currentChild)
			{
				// Get the next sibling from the component here in case the current child gets deleted in the DrawEntityNode() function
				Entity nextSibling = currentChild.GetComponent<SceneHierarchyComponent>().NextSibling;
				DrawEntityNode(currentChild);
				currentChild = nextSibling;
			}

			ImGui::TreePop();
		}

		if (removeEntity)
		{
			if (m_SelectedEntity == entity)
				m_SelectedEntity = {};

			m_Scene->DeleteEntity(entity);
		}
	}
}
