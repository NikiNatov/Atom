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
    Mesh::Mesh(const Vector<Vertex>& vertices, const Vector<u32>& indices, const Vector<Submesh>& submeshes, const MaterialTable& materialTable, bool isReadable)
        : Asset(AssetType::Mesh), m_Submeshes(submeshes), m_MaterialTable(materialTable), m_IsReadable(isReadable)
    {
        BufferDescription vbDesc;
        vbDesc.ElementCount = vertices.size();
        vbDesc.ElementSize = sizeof(Vertex);
        vbDesc.IsDynamic = false;

        m_VertexBuffer = CreateRef<VertexBuffer>(vbDesc, "VB");
        Renderer::UploadBufferData(vertices.data(), m_VertexBuffer.get());

        BufferDescription ibDesc;
        ibDesc.ElementCount = indices.size();
        ibDesc.ElementSize = sizeof(u32);
        ibDesc.IsDynamic = false;

        m_IndexBuffer = CreateRef<IndexBuffer>(ibDesc, IndexBufferFormat::U32, "IB");
        Renderer::UploadBufferData(indices.data(), m_IndexBuffer.get());

        if (m_IsReadable)
        {
            m_Vertices = vertices;
            m_Indices = indices;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Mesh::UpdateGPUData(bool makeNonReadable)
    {
        if (m_IsReadable)
        {
            m_IsReadable = !makeNonReadable;

            if (m_VertexBuffer->GetElementCount() != m_Vertices.size())
            {
                BufferDescription vbDesc;
                vbDesc.ElementCount = m_Vertices.size();
                vbDesc.ElementSize = sizeof(Vertex);
                vbDesc.IsDynamic = false;

                m_VertexBuffer = CreateRef<VertexBuffer>(vbDesc, "VB");
            }

            if (m_IndexBuffer->GetElementCount() != m_Indices.size())
            {
                BufferDescription ibDesc;
                ibDesc.ElementCount = m_Indices.size();
                ibDesc.ElementSize = sizeof(u32);
                ibDesc.IsDynamic = false;

                m_IndexBuffer = CreateRef<IndexBuffer>(ibDesc, IndexBufferFormat::U32, "IB");
            }

            Renderer::UploadBufferData(m_Vertices.data(), m_VertexBuffer.get());
            Renderer::UploadBufferData(m_Indices.data(), m_VertexBuffer.get());

            if (makeNonReadable)
            {
                m_Vertices.clear();
                m_Indices.clear();
            }
        }
    }
}
