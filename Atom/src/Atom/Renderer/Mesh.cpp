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
    Mesh::Mesh(const Vector<Vertex>& vertices, const Vector<u32>& indices, const Vector<Submesh>& submeshes, const Ref<MaterialTable>& materialTable, bool isReadable)
        : Asset(AssetType::Mesh), m_Submeshes(submeshes), m_MaterialTable(materialTable), m_IsReadable(isReadable)
    {
        if (!vertices.empty())
        {
            BufferDescription vbDesc;
            vbDesc.ElementCount = vertices.size();
            vbDesc.ElementSize = sizeof(Vertex);
            vbDesc.IsDynamic = false;

            m_VertexBuffer = CreateRef<VertexBuffer>(vbDesc, "VB");
            Renderer::UploadBufferData(vertices.data(), m_VertexBuffer.get());
        }

        if (!indices.empty())
        {
            BufferDescription ibDesc;
            ibDesc.ElementCount = indices.size();
            ibDesc.ElementSize = sizeof(u32);
            ibDesc.IsDynamic = false;

            m_IndexBuffer = CreateRef<IndexBuffer>(ibDesc, IndexBufferFormat::U32, "IB");
            Renderer::UploadBufferData(indices.data(), m_IndexBuffer.get());
        }

        if (m_IsReadable)
        {
            m_Vertices = vertices;
            m_Indices = indices;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Mesh::Mesh(Mesh&& rhs) noexcept
        : Asset(AssetType::Mesh), 
        m_Vertices(std::move(rhs.m_Vertices)), m_Indices(std::move(rhs.m_Indices)), 
        m_Submeshes(std::move(rhs.m_Submeshes)), m_MaterialTable(std::move(rhs.m_MaterialTable)),
        m_VertexBuffer(std::move(rhs.m_VertexBuffer)), m_IndexBuffer(std::move(rhs.m_IndexBuffer)), m_IsReadable(rhs.m_IsReadable)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Mesh& Mesh::operator=(Mesh&& rhs) noexcept
    {
        if (this != &rhs)
        {
            m_Vertices = std::move(rhs.m_Vertices);
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

            if (!m_VertexBuffer || m_VertexBuffer->GetElementCount() != m_Vertices.size())
            {
                if (m_Vertices.size() != 0)
                {
                    BufferDescription vbDesc;
                    vbDesc.ElementCount = m_Vertices.size();
                    vbDesc.ElementSize = sizeof(Vertex);
                    vbDesc.IsDynamic = false;

                    m_VertexBuffer = CreateRef<VertexBuffer>(vbDesc, "VB");
                }
                else
                {
                    m_VertexBuffer = nullptr;
                }
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

            if(m_Vertices.size())
                Renderer::UploadBufferData(m_Vertices.data(), m_VertexBuffer.get());

            if(m_Indices.size())
                Renderer::UploadBufferData(m_Indices.data(), m_IndexBuffer.get());

            if (makeNonReadable)
            {
                m_Vertices.clear();
                m_Indices.clear();
            }
        }
    }
}
