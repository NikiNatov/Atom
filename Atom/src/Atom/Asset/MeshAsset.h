#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Asset/SkeletonAsset.h"
#include "Atom/Asset/MaterialAsset.h"

#include <glm/glm.hpp>

namespace Atom
{
    struct BoneWeight
    {
        std::pair<u32, f32> Weights[Skeleton::Bone::MAX_BONE_WEIGHTS];
    };

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec2 TexCoord;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
    };

    struct AnimatedVertex
    {
        glm::vec3 Position;
        glm::vec2 TexCoord;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
        u32       BoneIDs[Skeleton::Bone::MAX_BONE_WEIGHTS];
        f32       BoneWeights[Skeleton::Bone::MAX_BONE_WEIGHTS];
    };

    struct Submesh
    {
        u32 StartVertex;
        u32 VertexCount;
        u32 StartIndex;
        u32 IndexCount;
        u32 MaterialIndex;
    };

    struct MeshDescription
    {
        Vector<glm::vec3>  Positions;
        Vector<glm::vec2>  UVs;
        Vector<glm::vec3>  Normals;
        Vector<glm::vec3>  Tangents;
        Vector<glm::vec3>  Bitangents;
        Vector<BoneWeight> BoneWeights;
        Vector<u32>        Indices;
        Vector<Submesh>    Submeshes;
        Ref<MaterialTable> MaterialTable;
    };

    class Mesh : public Asset
    {
        friend class AssetSerializer;
        friend class ContentTools;
    public:
        Mesh();
        Mesh(const MeshDescription& desc, bool isReadable);

        Mesh(const Mesh& rhs) = delete;
        Mesh& operator=(const Mesh& rhs) = delete;

        Mesh(Mesh&& rhs) noexcept;
        Mesh& operator=(Mesh&& rhs) noexcept;

        void UpdateGPUData(bool makeNonReadable = false);

        inline void SetPositions(const Vector<glm::vec3>& positions) { m_Positions = positions; }
        inline void SetUVs(const Vector<glm::vec2>& uvs) { m_UVs = uvs; }
        inline void SetNormals(const Vector<glm::vec3>& normals) { m_Normals = normals; }
        inline void SetTangents(const Vector<glm::vec3>& tangents) { m_Tangents = tangents; }
        inline void SetBitangents(const Vector<glm::vec3>& bitangents) { m_Bitangents = bitangents; }
        inline void SetBoneWeights(const Vector<BoneWeight>& boneWeights) { m_BoneWeights = boneWeights; }
        inline void SetIndices(const Vector<u32>& indices) { m_Indices = indices; }
        inline void SetSubmeshes(const Vector<Submesh>& submeshes) { m_Submeshes = submeshes; }
        inline void SetMaterial(u32 submeshIdx, Ref<MaterialAsset> material) { ATOM_ENGINE_ASSERT(submeshIdx < m_Submeshes.size()); m_MaterialTable->SetMaterial(submeshIdx, material); }

        inline const Vector<glm::vec3>& GetPositions() const { return m_Positions; }
        inline const Vector<glm::vec2>& GetUVs() const { return m_UVs; }
        inline const Vector<glm::vec3>& GetNormals() const { return m_Normals; }
        inline const Vector<glm::vec3>& GetTangents() const { return m_Tangents; }
        inline const Vector<glm::vec3>& GetBitangents() const { return m_Bitangents; }
        inline const Vector<BoneWeight>& GetBoneWeights() const { return m_BoneWeights; }
        inline const Vector<u32>& GetIndices() const { return m_Indices; }
        inline const Vector<Submesh>& GetSubmeshes() const { return m_Submeshes; }
        inline const Ref<MaterialAsset> GetMaterial(u32 submeshIdx) const { ATOM_ENGINE_ASSERT(submeshIdx < m_Submeshes.size()); return m_MaterialTable->GetMaterial(submeshIdx); }
        inline const Ref<MaterialTable>& GetMaterialTable() const { return m_MaterialTable; }
        inline bool IsReadable() const { return m_IsReadable; }
        inline bool IsEmpty() const { return !m_VertexBuffer || !m_IndexBuffer || !m_Submeshes.size(); }
        inline Ref<VertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
        inline Ref<IndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }
    private:
        Vector<glm::vec3>  m_Positions;
        Vector<glm::vec2>  m_UVs;
        Vector<glm::vec3>  m_Normals;
        Vector<glm::vec3>  m_Tangents;
        Vector<glm::vec3>  m_Bitangents;
        Vector<BoneWeight> m_BoneWeights;
        Vector<u32>        m_Indices;
        Vector<Submesh>    m_Submeshes;
        Ref<MaterialTable> m_MaterialTable;
        bool               m_IsReadable = true;
        Ref<VertexBuffer>  m_VertexBuffer = nullptr;
        Ref<IndexBuffer>   m_IndexBuffer = nullptr;
    };
}