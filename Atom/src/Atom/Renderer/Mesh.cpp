#include "atompch.h"
#include "Mesh.h"

#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Renderer.h"

#include "Atom/Tools/ContentTools.h"
#include "Atom/Asset/AssetManager.h"
#include "Atom/Asset/AssetSerializer.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Mesh::Mesh()
        : Asset(AssetType::Mesh)
    {
        m_MaterialTable = CreateRef<MaterialTable>();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Mesh::Mesh(const MeshDescription& desc, bool isReadable)
        : Asset(AssetType::Mesh), m_Submeshes(desc.Submeshes), m_MaterialTable(desc.MaterialTable), m_IsReadable(isReadable)
    {
        if (!desc.BoneWeights.empty())
        {
            // Animated meshes
            Vector<AnimatedVertex> vertices;
            vertices.reserve(desc.Positions.size());

            for (u32 i = 0; i < desc.Positions.size(); i++)
            {
                AnimatedVertex v;
                v.Position = desc.Positions[i];
                v.TexCoord = desc.UVs[i];
                v.Normal = desc.Normals[i];
                v.Tangent = desc.Tangents[i];
                v.Bitangent = desc.Bitangents[i];
                for (u32 weightIdx = 0; weightIdx < Skeleton::Bone::MAX_BONE_WEIGHTS; weightIdx++)
                {
                    auto& [boneID, weight] = desc.BoneWeights[i].Weights[weightIdx];
                    v.BoneIDs[weightIdx] = boneID;
                    v.BoneWeights[weightIdx] = weight;
                }
                vertices.push_back(std::move(v));
            }

            if (!vertices.empty())
            {
                BufferDescription vbDesc;
                vbDesc.ElementCount = vertices.size();
                vbDesc.ElementSize = sizeof(AnimatedVertex);
                vbDesc.IsDynamic = false;

                m_VertexBuffer = CreateRef<VertexBuffer>(vbDesc, "VB");
                Renderer::UploadBufferData(vertices.data(), m_VertexBuffer.get());
            }
        }
        else
        {
            // Regular meshes
            Vector<Vertex> vertices;
            vertices.reserve(desc.Positions.size());

            for (u32 i = 0; i < desc.Positions.size(); i++)
            {
                Vertex v;
                v.Position = desc.Positions[i];
                v.TexCoord = desc.UVs[i];
                v.Normal = desc.Normals[i];
                v.Tangent = desc.Tangents[i];
                v.Bitangent = desc.Bitangents[i];
                vertices.push_back(std::move(v));
            }

            if (!vertices.empty())
            {
                BufferDescription vbDesc;
                vbDesc.ElementCount = vertices.size();
                vbDesc.ElementSize = sizeof(Vertex);
                vbDesc.IsDynamic = false;

                m_VertexBuffer = CreateRef<VertexBuffer>(vbDesc, "VB");
                Renderer::UploadBufferData(vertices.data(), m_VertexBuffer.get());
            }
        }

        if (!desc.Indices.empty())
        {
            BufferDescription ibDesc;
            ibDesc.ElementCount = desc.Indices.size();
            ibDesc.ElementSize = sizeof(u32);
            ibDesc.IsDynamic = false;

            m_IndexBuffer = CreateRef<IndexBuffer>(ibDesc, IndexBufferFormat::U32, "IB");
            Renderer::UploadBufferData(desc.Indices.data(), m_IndexBuffer.get());
        }

        if (m_IsReadable)
        {
            m_Positions = desc.Positions;
            m_UVs = desc.UVs;
            m_Normals = desc.Normals;
            m_Tangents = desc.Tangents;
            m_Bitangents = desc.Bitangents;
            m_BoneWeights = desc.BoneWeights;
            m_Indices = desc.Indices;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Mesh::Mesh(Mesh&& rhs) noexcept
        : Asset(AssetType::Mesh), 
        m_Positions(std::move(rhs.m_Positions)), m_UVs(std::move(rhs.m_UVs)), m_Normals(std::move(rhs.m_Normals)), m_Tangents(std::move(rhs.m_Tangents)), m_Bitangents(std::move(rhs.m_Bitangents)),
        m_BoneWeights(std::move(rhs.m_BoneWeights)),
        m_Indices(std::move(rhs.m_Indices)),
        m_Submeshes(std::move(rhs.m_Submeshes)), m_MaterialTable(std::move(rhs.m_MaterialTable)),
        m_VertexBuffer(std::move(rhs.m_VertexBuffer)), m_IndexBuffer(std::move(rhs.m_IndexBuffer)), m_IsReadable(rhs.m_IsReadable)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Mesh& Mesh::operator=(Mesh&& rhs) noexcept
    {
        if (this != &rhs)
        {
            m_Positions = std::move(rhs.m_Positions);
            m_UVs = std::move(rhs.m_UVs);
            m_Normals = std::move(rhs.m_Normals);
            m_Tangents = std::move(rhs.m_Tangents);
            m_Bitangents = std::move(rhs.m_Bitangents);
            m_BoneWeights = std::move(rhs.m_BoneWeights);
            m_Indices = std::move(rhs.m_Indices);
            m_Submeshes = std::move(rhs.m_Submeshes);
            m_MaterialTable = std::move(rhs.m_MaterialTable);
            m_VertexBuffer = std::move(rhs.m_VertexBuffer);
            m_IndexBuffer = std::move(rhs.m_IndexBuffer);
            m_IsReadable = rhs.m_IsReadable;
        }

        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Mesh::UpdateGPUData(bool makeNonReadable)
    {
        if (m_IsReadable)
        {
            m_IsReadable = !makeNonReadable;

            if (!m_BoneWeights.empty())
            {
                // Animated meshes
                Vector<AnimatedVertex> vertices;
                vertices.reserve(m_Positions.size());

                for (u32 i = 0; i < m_Positions.size(); i++)
                {
                    AnimatedVertex v;
                    v.Position = m_Positions[i];
                    v.TexCoord = m_UVs[i];
                    v.Normal = m_Normals[i];
                    v.Tangent = m_Tangents[i];
                    v.Bitangent = m_Bitangents[i];
                    for (u32 weightIdx = 0; weightIdx < Skeleton::Bone::MAX_BONE_WEIGHTS; weightIdx++)
                    {
                        auto& [boneID, weight] = m_BoneWeights[i].Weights[weightIdx];
                        v.BoneIDs[weightIdx] = boneID;
                        v.BoneWeights[weightIdx] = weight;
                    }
                    vertices.push_back(std::move(v));
                }

                if (!m_VertexBuffer || m_VertexBuffer->GetElementCount() != vertices.size())
                {
                    if (vertices.size() != 0)
                    {
                        BufferDescription vbDesc;
                        vbDesc.ElementCount = vertices.size();
                        vbDesc.ElementSize = sizeof(AnimatedVertex);
                        vbDesc.IsDynamic = false;

                        m_VertexBuffer = CreateRef<VertexBuffer>(vbDesc, "VB");
                    }
                    else
                    {
                        m_VertexBuffer = nullptr;
                    }
                }

                if (vertices.size())
                    Renderer::UploadBufferData(vertices.data(), m_VertexBuffer.get());
            }
            else
            {
                // Regular meshes
                Vector<Vertex> vertices;
                vertices.reserve(m_Positions.size());

                for (u32 i = 0; i < m_Positions.size(); i++)
                {
                    Vertex v;
                    v.Position = m_Positions[i];
                    v.TexCoord = m_UVs[i];
                    v.Normal = m_Normals[i];
                    v.Tangent = m_Tangents[i];
                    v.Bitangent = m_Bitangents[i];
                    vertices.push_back(std::move(v));
                }

                if (!m_VertexBuffer || m_VertexBuffer->GetElementCount() != vertices.size())
                {
                    if (vertices.size() != 0)
                    {
                        BufferDescription vbDesc;
                        vbDesc.ElementCount = vertices.size();
                        vbDesc.ElementSize = sizeof(Vertex);
                        vbDesc.IsDynamic = false;

                        m_VertexBuffer = CreateRef<VertexBuffer>(vbDesc, "VB");
                    }
                    else
                    {
                        m_VertexBuffer = nullptr;
                    }
                }

                if (vertices.size())
                    Renderer::UploadBufferData(vertices.data(), m_VertexBuffer.get());
            }

            if (!m_IndexBuffer || m_IndexBuffer->GetElementCount() != m_Indices.size())
            {
                if (m_Indices.size() != 0)
                {
                    BufferDescription ibDesc;
                    ibDesc.ElementCount = m_Indices.size();
                    ibDesc.ElementSize = sizeof(u32);
                    ibDesc.IsDynamic = false;

                    m_IndexBuffer = CreateRef<IndexBuffer>(ibDesc, IndexBufferFormat::U32, "IB");
                }
                else
                {
                    m_IndexBuffer = nullptr;
                }
            }

            if(m_Indices.size())
                Renderer::UploadBufferData(m_Indices.data(), m_IndexBuffer.get());

            if (makeNonReadable)
            {
                m_Positions.clear();
                m_UVs.clear();
                m_Normals.clear();
                m_Tangents.clear();
                m_Bitangents.clear();
                m_BoneWeights.clear();
                m_Indices.clear();
            }
        }
    }
}
