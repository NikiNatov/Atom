#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Mesh.h"

#include "Atom/Scripting/ScriptWrappers/Renderer/MaterialWrapper.h"

namespace Atom
{
    namespace ScriptWrappers
    {
        class Mesh
        {
        public:
            Mesh();
            Mesh(const Ref<Atom::Mesh>& mesh);

            void UpdateGPUData(bool makeNonReadable = false);

            void SetVertices(const Vector<Vertex>& vertices);
            void SetIndices(const Vector<u32>& indices);
            void SetSubmeshes(const Vector<Submesh>& submeshes);
            void SetMaterial(u32 submeshIdx, Material material);

            bool IsReadable() const;
            bool IsEmpty() const;
            const Vector<Vertex>& GetVertices() const;
            const Vector<u32>& GetIndices() const;
            const Vector<Submesh>& GetSubmeshes() const;
            Material GetMaterial(u32 submeshIdx) const;

            Ref<Atom::Mesh> GetMesh() const;

            static Mesh Find(const std::filesystem::path& assetPath);
        private:
            Ref<Atom::Mesh> m_Mesh = nullptr;
        };
    }
}