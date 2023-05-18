#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/MeshAsset.h"

#include "Atom/Scripting/ScriptWrappers/Renderer/MaterialWrapper.h"

#include <pybind11/pybind11.h>

namespace Atom
{
    namespace ScriptWrappers
    {
        class Mesh
        {
        public:
            Mesh();
            Mesh(u64 assetUUID);
            Mesh(const Ref<Atom::Mesh>& mesh);

            void UpdateGPUData(bool makeNonReadable = false);

            void SetPositions(const pybind11::list& positions);
            void SetUVs(const pybind11::list& uvs);
            void SetNormals(const pybind11::list& normals);
            void SetTangents(const pybind11::list& tangents);
            void SetBitangents(const pybind11::list& bitangents);
            void SetIndices(const pybind11::list& indices);
            void SetSubmeshes(const pybind11::list& submeshes);
            void SetMaterial(u32 submeshIdx, Material material);

            bool IsReadable() const;
            bool IsEmpty() const;
            const pybind11::list& GetPositions() const;
            const pybind11::list& GetUVs() const;
            const pybind11::list& GetNormals() const;
            const pybind11::list& GetTangents() const;
            const pybind11::list& GetBitangents() const;
            const pybind11::list& GetIndices() const;
            const pybind11::list& GetSubmeshes() const;
            Material GetMaterial(u32 submeshIdx) const;

            UUID GetUUID() const;
            Ref<Atom::Mesh> GetMesh() const;

            static Mesh Find(const std::filesystem::path& assetPath);
        private:
            Ref<Atom::Mesh> m_Mesh = nullptr;
        };
    }
}