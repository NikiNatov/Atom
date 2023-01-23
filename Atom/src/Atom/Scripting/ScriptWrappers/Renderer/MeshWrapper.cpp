#include "atompch.h"
#include "MeshWrapper.h"

#include "Atom/Asset/AssetManager.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        // -----------------------------------------------------------------------------------------------------------------------------
        Mesh::Mesh()
        {
            m_Mesh = CreateRef<Atom::Mesh>();
            AssetManager::RegisterAsset(m_Mesh);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Mesh::Mesh(u64 assetUUID)
            : m_Mesh(nullptr)
        {
            if (assetUUID != 0)
                m_Mesh = AssetManager::GetAsset<Atom::Mesh>(assetUUID, true);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Mesh::Mesh(const Ref<Atom::Mesh>& mesh)
            : m_Mesh(mesh)
        {
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Mesh::UpdateGPUData(bool makeNonReadable)
        {
            if (m_Mesh)
            {
                m_Mesh->UpdateGPUData(makeNonReadable);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Mesh::SetVertices(const Vector<Vertex>& vertices)
        {
            if (m_Mesh)
            {
                m_Mesh->SetVertices(vertices);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Mesh::SetIndices(const Vector<u32>& indices)
        {
            if (m_Mesh)
            {
                m_Mesh->SetIndices(indices);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Mesh::SetSubmeshes(const Vector<Submesh>& submeshes)
        {
            if (m_Mesh)
            {
                m_Mesh->SetSubmeshes(submeshes);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Mesh::SetMaterial(u32 submeshIdx, Material material)
        {
            if (m_Mesh)
            {
                if (submeshIdx < m_Mesh->GetSubmeshes().size())
                {
                    m_Mesh->SetMaterial(submeshIdx, material.GetMaterial());
                }
                else
                {
                    ATOM_ERROR("Invalid submesh index used when trying to set material");
                }
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Mesh::IsReadable() const
        {
            if (m_Mesh)
            {
                return m_Mesh->IsReadable();
            }

            return false;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        bool Mesh::IsEmpty() const
        {
            if (m_Mesh)
            {
                return m_Mesh->IsEmpty();
            }

            return true;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const Vector<Vertex>& Mesh::GetVertices() const
        {
            if (m_Mesh)
            {
                return m_Mesh->GetVertices();
            }

            return {};
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const Vector<u32>& Mesh::GetIndices() const
        {
            if (m_Mesh)
            {
                return m_Mesh->GetIndices();
            }

            return {};
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const Vector<Submesh>& Mesh::GetSubmeshes() const
        {
            if (m_Mesh)
            {
                return m_Mesh->GetSubmeshes();
            }

            return {};
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Material Mesh::GetMaterial(u32 submeshIdx) const
        {
            if (m_Mesh)
            {
                return Material(m_Mesh->GetMaterial(submeshIdx));
            }

            return Material(nullptr);
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        UUID Mesh::GetUUID() const
        {
            return m_Mesh ? m_Mesh->GetUUID() : 0;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Ref<Atom::Mesh> Mesh::GetMesh() const
        {
            return m_Mesh;
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        Mesh Mesh::Find(const std::filesystem::path& assetPath)
        {
            UUID uuid = AssetManager::GetUUIDForAssetPath(AssetManager::GetAssetsFolder() / assetPath);

            if (uuid == 0)
                return Mesh(nullptr);

            return Mesh(AssetManager::GetAsset<Atom::Mesh>(uuid, true));
        }
    }
}