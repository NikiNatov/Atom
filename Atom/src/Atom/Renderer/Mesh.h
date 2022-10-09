#pragma once

#include "Atom/Core/Core.h"

#include <glm/glm.hpp>

namespace Atom
{
    class VertexBuffer;
    class IndexBuffer;
    class Material;

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec2 TexCoord;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
    };

    struct Submesh
    {
        u32 StartVertex;
        u32 VertexCount;
        u32 StartIndex;
        u32 IndexCount;
        u32 MaterialIndex;
    };

    class Mesh
    {
    public:
        Mesh(const String& name, const Vector<Vertex>& vertices, const Vector<u32>& indices, const Vector<Submesh>& submeshes, const Vector<Ref<Material>>& materials);
        Mesh(const std::filesystem::path& filepath);

        inline const String& GetName() const { return m_Name; }
        inline const Ref<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
        inline const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
        inline const Vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }
        inline const Vector<Ref<Material>>& GetMaterials() const { return m_Materials; }
    private:
        void CreateBuffers(const Vector<Vertex>& vertices, const Vector<u32>& indices);
    private:
        String                m_Name;
        Ref<VertexBuffer>     m_VertexBuffer;
        Ref<IndexBuffer>      m_IndexBuffer;
        Vector<Submesh>       m_Submeshes;
        Vector<Ref<Material>> m_Materials;
    };
}