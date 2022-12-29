#include "AssetManagerPanel.h"

#include "Atom/Asset/AssetManager.h"
#include <imgui.h>

namespace Atom
{
    static String AssetTypeToString(AssetType type)
    {
        switch (type)
        {
            case AssetType::Texture2D:      return "Texture2D";
            case AssetType::TextureCube:    return "TextureCube";
            case AssetType::Mesh:           return "Mesh";
            case AssetType::Material:       return "Material";
        }

        ATOM_ENGINE_ASSERT(false, "Unknown asset type");
        return "";
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetManagerPanel::OnImGuiRender()
    {
        ImGui::Begin("Asset Manager");

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_AllowItemOverlap;
        if (ImGui::TreeNodeEx("Registered assets", flags))
        {
            const auto& assetRegistry = AssetManager::GetRegistry();
            for (auto& [uuid, assetMetaData] : assetRegistry)
            {
                ImGui::PushID(uuid);
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 60.0f);
                ImGui::Text("UUID");
                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);

                bool isAssetLoaded = AssetManager::IsAssetLoaded(uuid);

                if (isAssetLoaded)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.0f, 1.0f, 0.0f, 1.0f });

                ImGui::InputScalar("##UUID", ImGuiDataType_U64, (void*)&uuid, 0, 0, 0, ImGuiInputTextFlags_ReadOnly);

                if (isAssetLoaded)
                    ImGui::PopStyleColor();

                ImGui::PopItemWidth();
                ImGui::Columns(1);

                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 60.0f);
                ImGui::Text("Path");
                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);
                String inputText = (assetMetaData.Flags & AssetFlags::Serialized) != AssetFlags::None ? assetMetaData.AssetFilepath.string() : "<Virtual Asset>";
                ImGui::InputText("##Path", inputText.data(), inputText.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::PopItemWidth();
                ImGui::Columns(1);

                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 60.0f);
                ImGui::Text("Type");
                ImGui::NextColumn();
                ImGui::PushItemWidth(-1);
                String type = AssetTypeToString(assetMetaData.Type);
                ImGui::InputText("##Type", type.data(), type.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::PopItemWidth();
                ImGui::Columns(1);

                ImGui::PopID();
                ImGui::Separator();
            }

            ImGui::TreePop();
        }

        ImGui::End();
    }
}
