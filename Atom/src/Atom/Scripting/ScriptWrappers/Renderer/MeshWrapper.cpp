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
        void Mesh::SetPositions(const pybind11::list& positions)
        {
            if (m_Mesh)
            {
                Vector<glm::vec3> vec;
                vec.reserve(positions.size());

                for (auto& elm : positions)
                {
                    auto tuple = elm.cast<pybind11::tuple>();
                    vec.emplace_back(tuple[0].cast<f32>(), tuple[1].cast<f32>(), tuple[2].cast<f32>());
                }

                m_Mesh->SetPositions(vec);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Mesh::SetUVs(const pybind11::list& uvs)
        {
            if (m_Mesh)
            {
                Vector<glm::vec2> vec;
                vec.reserve(uvs.size());

                for (auto& elm : uvs)
                {
                    auto tuple = elm.cast<pybind11::tuple>();
                    vec.emplace_back(tuple[0].cast<f32>(), tuple[1].cast<f32>());
                }

                m_Mesh->SetUVs(vec);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Mesh::SetNormals(const pybind11::list& normals)
        {
            if (m_Mesh)
            {
                Vector<glm::vec3> vec;
                vec.reserve(normals.size());

                for (auto& elm : normals)
                {
                    auto tuple = elm.cast<pybind11::tuple>();
                    vec.emplace_back(tuple[0].cast<f32>(), tuple[1].cast<f32>(), tuple[2].cast<f32>());
                }

                m_Mesh->SetNormals(vec);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Mesh::SetTangents(const pybind11::list& tangents)
        {
            if (m_Mesh)
            {
                Vector<glm::vec3> vec;
                vec.reserve(tangents.size());

                for (auto& elm : tangents)
                {
                    auto tuple = elm.cast<pybind11::tuple>();
                    vec.emplace_back(tuple[0].cast<f32>(), tuple[1].cast<f32>(), tuple[2].cast<f32>());
                }

                m_Mesh->SetTangents(vec);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Mesh::SetBitangents(const pybind11::list& bitangents)
        {
            if (m_Mesh)
            {
                Vector<glm::vec3> vec;
                vec.reserve(bitangents.size());

                for (auto& elm : bitangents)
                {
                    auto tuple = elm.cast<pybind11::tuple>();
                    vec.emplace_back(tuple[0].cast<f32>(), tuple[1].cast<f32>(), tuple[2].cast<f32>());
                }

                m_Mesh->SetBitangents(vec);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Mesh::SetIndices(const pybind11::list& indices)
        {
            if (m_Mesh)
            {
                Vector<u32> vec;
                vec.reserve(indices.size());

                for (auto& elm : indices)
                {
                    vec.emplace_back(elm.cast<u32>());
                }

                m_Mesh->SetIndices(vec);
            }
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        void Mesh::SetSubmeshes(const pybind11::list& submeshes)
        {
            if (m_Mesh)
            {
                Vector<Atom::Submesh> vec;
                vec.reserve(submeshes.size());

                for (auto& elm : submeshes)
                {
                    vec.emplace_back(elm.cast<Atom::Submesh>());
                }

                m_Mesh->SetSubmeshes(vec);
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
        const pybind11::list& Mesh::GetPositions() const
        {
            if (m_Mesh)
            {
                return pybind11::cast(m_Mesh->GetPositions());
            }

            return pybind11::none();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const pybind11::list& Mesh::GetUVs() const
        {
            if (m_Mesh)
            {
                return pybind11::cast(m_Mesh->GetUVs());
            }

            return pybind11::none();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const pybind11::list& Mesh::GetNormals() const
        {
            if (m_Mesh)
            {
                return pybind11::cast(m_Mesh->GetNormals());
            }

            return pybind11::none();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const pybind11::list& Mesh::GetTangents() const
        {
            if (m_Mesh)
            {
                return pybind11::cast(m_Mesh->GetTangents());
            }

            return pybind11::none();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const pybind11::list& Mesh::GetBitangents() const
        {
            if (m_Mesh)
            {
                return pybind11::cast(m_Mesh->GetBitangents());
            }

            return pybind11::none();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const pybind11::list& Mesh::GetIndices() const
        {
            if (m_Mesh)
            {
                return pybind11::cast(m_Mesh->GetIndices());
            }

            return pybind11::none();
        }

        // -----------------------------------------------------------------------------------------------------------------------------
        const pybind11::list& Mesh::GetSubmeshes() const
        {
            if (m_Mesh)
            {
                return pybind11::cast(m_Mesh->GetSubmeshes());
            }

            return pybind11::none();
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