#include "atompch.h"
#include "Mesh.h"

#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/CommandQueue.h"
#include "Atom/Renderer/CommandBuffer.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/Renderer.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Mesh::Mesh(const String& name, const Vector<Vertex>& vertices, const Vector<u32>& indices, const Vector<Submesh>& submeshes, const Vector<Ref<Material>>& materials)
        : m_Name(name), m_Submeshes(submeshes), m_Materials(materials)
    {
        CreateBuffers(vertices, indices);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Mesh::Mesh(const std::filesystem::path& filepath)
    {
        u32 processingFlags = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_PreTransformVertices;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath.string().c_str(), processingFlags);

        m_Name = scene->mName.C_Str();
        m_Materials.reserve(scene->mNumMaterials);
        m_Submeshes.reserve(scene->mNumMeshes);

        Vector<Vertex> vertices;
        vertices.reserve(5000);

        Vector<u32> indices;
        indices.reserve(10000);

        // Parse all submeshes
        for (u32 submeshIdx = 0; submeshIdx < scene->mNumMeshes; submeshIdx++)
        {
            aiMesh* submesh = scene->mMeshes[submeshIdx];

            u32 startVertex = vertices.size();
            u32 startIndex = indices.size();

            // Construct all vertices
            u32 vertexCount = 0;
            for (u32 vertexIdx = 0; vertexIdx < submesh->mNumVertices; vertexIdx++)
            {
                const aiVector3D& position = submesh->mVertices[vertexIdx];
                const aiVector3D& texCoord = submesh->mTextureCoords[0][vertexIdx];
                const aiVector3D& normal = submesh->mNormals[vertexIdx];
                const aiVector3D& tangent = submesh->mTangents[vertexIdx];
                const aiVector3D& bitangent = submesh->mBitangents[vertexIdx];

                Vertex vertex;
                vertex.Position = { position.x, position.y, position.z };
                vertex.TexCoord = { texCoord.x, texCoord.y };
                vertex.Normal = { normal.x, normal.y, normal.z };
                vertex.Tangent = { tangent.x, tangent.y, tangent.z };
                vertex.Bitangent = { bitangent.x, bitangent.y, bitangent.z };

                vertices.push_back(vertex);
                vertexCount++;
            }

            // Construct all indices
            u32 indexCount = 0;
            for (u32 faceIdx = 0; faceIdx < submesh->mNumFaces; faceIdx++)
            {
                const aiFace& face = submesh->mFaces[faceIdx];
                for (u32 i = 0; i < face.mNumIndices; i++)
                {
                    indices.push_back(face.mIndices[i]);
                    indexCount++;
                }
            }

            // Create submesh
            Submesh sm;
            sm.StartVertex = startVertex;
            sm.VertexCount = vertexCount;
            sm.StartIndex = startIndex;
            sm.IndexCount = indexCount;
            sm.MaterialIndex = submesh->mMaterialIndex;

            m_Submeshes.push_back(sm);
        }

        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
        copyCommandBuffer->Begin();

        // Parse all materials
        for (u32 materialIdx = 0; materialIdx < scene->mNumMaterials; materialIdx++)
        {
            const aiMaterial* assimpMat = scene->mMaterials[materialIdx];
            
            Ref<GraphicsShader> shader = Renderer::GetShaderLibrary().Get<GraphicsShader>("Shader");

            MaterialFlags materialFlags = MaterialFlags::DepthTested;
            Ref<Material> material = CreateRef<Material>(shader, materialFlags, "UnnamedMaterial");

            // Set the name
            aiString name;
            if (assimpMat->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
            {
                material->SetName(name.C_Str());
            }

            // Set albedo color
            aiColor3D albedo;
            if (assimpMat->Get(AI_MATKEY_COLOR_DIFFUSE, albedo) == AI_SUCCESS)
            {
                // Set transparency flag
                f32 opacity = 1.0f;
                if (assimpMat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
                {
                    if (opacity < 1.0f)
                        materialFlags |= MaterialFlags::Transparent;
                }

                material->SetUniform("AlbedoColor", glm::vec4(albedo.r, albedo.g, albedo.b, opacity));
            }

            // Set two sided flag
            bool twoSided;
            if (assimpMat->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS && twoSided)
            {
                materialFlags |= MaterialFlags::TwoSided;
            }

            // Set wireframe flag
            bool wireframe;
            if (assimpMat->Get(AI_MATKEY_ENABLE_WIREFRAME, wireframe) == AI_SUCCESS && wireframe)
            {
                materialFlags |= MaterialFlags::Wireframe;
            }

            material->SetFlags(materialFlags);

            auto SetMaterialTexture = [&](aiTextureType type, const char* uniformName) 
            {
                aiString aiPath;
                if (assimpMat->GetTexture(type, 0, &aiPath) == AI_SUCCESS)
                {
                    std::unique_ptr<Image2D> image = nullptr;
                    String name = "UnnamedTexture";

                    if (const aiTexture* aiTexture = scene->GetEmbeddedTexture(aiPath.C_Str()))
                    {
                        // Texture is embedded. Decode the data buffer.
                        image = std::make_unique<Image2D>((byte*)(aiTexture->pcData), aiTexture->mWidth);
                        name = aiTexture->mFilename.C_Str();
                    }
                    else
                    {
                        // Load the texture from filepath
                        std::filesystem::path textureFilepath = filepath.parent_path() / "textures" / aiPath.C_Str();
                        image = std::make_unique<Image2D>(textureFilepath.string());
                        name = textureFilepath.filename().string();
                    }

                    TextureDescription textureDesc;
                    textureDesc.Width = image->GetWidth();
                    textureDesc.Height = image->GetHeight();
                    textureDesc.Format = TextureFormat::RGBA8;
                    textureDesc.MipLevels = image->GetMaxMipCount();

                    Ref<Texture2D> texture = CreateRef<Texture2D>(textureDesc, name.c_str());
                    copyCommandBuffer->UploadTextureData(image->GetPixelData().data(), texture.get());

                    material->SetTexture(uniformName, texture);
                    material->SetUniform(fmt::format("Use{}", uniformName).c_str(), 1);
                }
            };

            // Set albedo texture
            SetMaterialTexture(aiTextureType_DIFFUSE, "AlbedoTexture");
            
            m_Materials.push_back(material);
        }

        copyCommandBuffer->End();
        copyQueue->ExecuteCommandList(copyCommandBuffer);

        CreateBuffers(vertices, indices);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void Mesh::CreateBuffers(const Vector<Vertex>& vertices, const Vector<u32>& indices)
    {
        BufferDescription vbDesc;
        vbDesc.ElementCount = vertices.size();
        vbDesc.ElementSize = sizeof(Vertex);
        vbDesc.IsDynamic = false;

        m_VertexBuffer = CreateRef<VertexBuffer>(vbDesc, "VB");

        BufferDescription ibDesc;
        ibDesc.ElementCount = indices.size();
        ibDesc.ElementSize = sizeof(u32);
        ibDesc.IsDynamic = false;

        m_IndexBuffer = CreateRef<IndexBuffer>(ibDesc,IndexBufferFormat::U32, "IB");

        // Upload the data to the GPU
        CommandQueue* copyQueue = Device::Get().GetCommandQueue(CommandQueueType::Copy);
        Ref<CommandBuffer> copyCommandBuffer = copyQueue->GetCommandBuffer();
        copyCommandBuffer->Begin();
        copyCommandBuffer->UploadBufferData(vertices.data(), m_VertexBuffer.get());
        copyCommandBuffer->UploadBufferData(indices.data(), m_IndexBuffer.get());
        copyCommandBuffer->End();
        copyQueue->ExecuteCommandList(copyCommandBuffer);
    }
}
