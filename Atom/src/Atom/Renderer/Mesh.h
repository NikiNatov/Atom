#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Asset/Asset.h"

#include <glm/glm.hpp>

namespace Atom
{
    class VertexBuffer;
    class IndexBuffer;

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

    class Mesh : public Asset
    {
        friend class AssetSerializer;
        friend class ContentTools;
    public:
        Mesh();
        Mesh(const Vector<Vertex>& vertices, const Vector<u32>& indices, const Vector<Submesh>& submeshes, const Ref<MaterialTable>& materialTable, bool isReadable);

        Mesh(const Mesh& rhs) = delete;
        Mesh& operator=(const Mesh& rhs) = delete;

        Mesh(Mesh&& rhs) noexcept;
        Mesh& operator=(Mesh&& rhs) noexcept;

        void UpdateGPUData(bool makeNonReadable = false);

        inline void SetVertices(const Vector<Vertex>& vertices) { m_Vertices = vertices; }
        inline void SetIndices(const Vector<u32>& indices) { m_Indices = indices; }
        inline void SetSubmeshes(const Vector<Submesh>& submeshes) { m_Submeshes = submeshes; }
        inline void SetMaterialTable(const Ref<MaterialTable>& materialTable) { m_MaterialTable = materialTable; }

        inline const Vector<Vertex>& GetVertices() const { return m_Vertices; }
        inline const Vector<u32>& GetIndices() const { return m_Indices; }
        inline const Vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }
        inline const Ref<MaterialTable>& GetMaterialTable() const { return m_MaterialTable; }
        inline bool IsReadable() const { return m_IsReadable; }
        inline bool IsEmpty() const { return !m_VertexBuffer || !m_IndexBuffer || !m_Submeshes.size(); }
        inline Ref<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
        inline Ref<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }
    private:
        Vector<Vertex>     m_Vertices;
        Vector<u32>        m_Indices;
        Vector<Submesh>    m_Submeshes;
        Ref<MaterialTable> m_MaterialTable;
        bool               m_IsReadable = true;
        Ref<VertexBuffer>  m_VertexBuffer = nullptr;
        Ref<IndexBuffer>   m_IndexBuffer = nullptr;
    };
}