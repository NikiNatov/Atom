#include "SceneHierarchyPanel.h"

#include <imgui.h>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void SceneHierarchyPanel::OnImGuiRender()
    {
        ImGui::Begin("Scene Hierarchy");

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered())
		{
			ImGui::OpenPopup("##SceneHierarchyPopup");
		}

		if (ImGui::BeginPopupContextItem("##SceneHierarchyPopup"))
		{
			if (ImGui::MenuItem("New entity"))
				m_Scene->CreateEntity();

			ImGui::EndPopup();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_SRC_ENTITY"))
			{
				Entity srcEntity = *(Entity*)payload->Data;
				srcEntity.RemoveParent();
				srcEntity.GetComponent<SceneHierarchyComponent>().PreviousSibling = 0;
				srcEntity.GetComponent<SceneHierarchyComponent>().NextSibling = 0;
			}

			ImGui::EndDragDropTarget();
		}

        m_Scene->m_Registry.each([&](auto entityID)
        {
            Entity entity(entityID, m_Scene.get());

			if (entity && !entity.GetComponent<SceneHierarchyComponent>().Parent)
			{
				DrawEntityNode(entity);
			}
        });

        if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered())
            m_SelectedEntity = {};

		for (auto entity : m_EntitiesToDelete)
			m_Scene->DeleteEntity(entity);

		m_EntitiesToDelete.clear();

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

		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
			m_SelectedEntity = entity;

		ImGui::PushID((void*)(u64)(u32)entity);

		bool removeEntity = false;

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			ImGui::OpenPopup("##EntityPopup");
		}

		if (ImGui::BeginPopupContextItem("##EntityPopup"))
		{
			m_SelectedEntity = entity;

			if (ImGui::BeginMenu("New child"))
			{
				if (ImGui::MenuItem("Empty entity"))
				{
					Entity newEntity = m_Scene->CreateEntity();
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

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("DRAG_SRC_ENTITY", &entity, sizeof(Entity));
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_SRC_ENTITY"))
			{
				Entity srcEntity = *(Entity*)payload->Data;
				Entity srcEntityParent = m_Scene->FindEntityByUUID(srcEntity.GetComponent<SceneHierarchyComponent>().Parent);

				if (entity != srcEntityParent && !entity.IsDescendantOf(srcEntity))
				{
					if (srcEntityParent)
						srcEntityParent.RemoveChild(srcEntity);

					entity.AddChild(srcEntity);
				}
			}
			
			ImGui::EndDragDropTarget();
		}

		ImGui::PopID();

		if (isOpen)
		{
			Entity currentChild = m_Scene->FindEntityByUUID(entity.GetComponent<SceneHierarchyComponent>().FirstChild);

			while (currentChild)
			{
				// Get the next sibling from the component here in case the current child gets deleted in the DrawEntityNode() function
				Entity nextSibling = m_Scene->FindEntityByUUID(currentChild.GetComponent<SceneHierarchyComponent>().NextSibling);
				DrawEntityNode(currentChild);
				currentChild = nextSibling;
			}

			ImGui::TreePop();
		}

		if (removeEntity)
		{
			if (m_SelectedEntity == entity)
				m_SelectedEntity = {};

			m_EntitiesToDelete.push_back(entity);
		}
	}
}
