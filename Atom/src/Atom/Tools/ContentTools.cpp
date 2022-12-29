#include "atompch.h"
#include "ContentTools.h"

#include "Atom/Asset/AssetSerializer.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/Mesh.h"
#include "Atom/Renderer/Renderer.h"

#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Atom
{
    namespace Utils
    {
        static u32 GetDesiredChannelsFromFormat(TextureFormat format)
        {
            switch (format)
            {
                case TextureFormat::R8:
                    return 1;
                case TextureFormat::RG16F:
                case TextureFormat::RG32F:
                    return 2;
                case TextureFormat::RGBA8:
                case TextureFormat::RGBA16F:
                case TextureFormat::RGBA32F:
                    return 4;
            }

            return 0;
        }

        static bool IsHDRFormat(TextureFormat format)
        {
            switch (format)
            {
                case TextureFormat::RG16F:
                case TextureFormat::RG32F:
                case TextureFormat::RGBA16F:
                case TextureFormat::RGBA32F:
                    return true;
            }

            return false;
        }

        static TextureFormat GetHDRFormatFromRegularFormat(TextureFormat format)
        {
            switch (format)
            {
                case TextureFormat::RG16F:   return TextureFormat::RG16F;
                case TextureFormat::RG32F:   return TextureFormat::RG32F;
                case TextureFormat::RGBA16F: return TextureFormat::RGBA16F;
                case TextureFormat::RGBA32F: return TextureFormat::RGBA32F;
                case TextureFormat::RGBA8:   return TextureFormat::RGBA32F;
            }

            return TextureFormat::RGBA32F;
        }

        static u32 GetFormatByteSize(TextureFormat format)
        {
            switch (format)
            {
                case TextureFormat::RG16F:   return 2 * 2;
                case TextureFormat::RG32F:   return 2 * 4;
                case TextureFormat::RGBA16F: return 4 * 2;
                case TextureFormat::RGBA32F: return 4 * 4;
                case TextureFormat::R8:      return 1 * 1;
                case TextureFormat::RGBA8:   return 4 * 1;
            }

            return 0;
        }

        static u32 GetMaxMipCount(u32 width, u32 height)
        {
            return (u32)glm::log2((f32)glm::max(width, height)) + 1;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID ContentTools::ImportTextureAsset(const std::filesystem::path& sourcePath, const std::filesystem::path& destinationFolder, const TextureImportSettings& importSettings)
    {
        String assetFilename = sourcePath.stem().string() + Asset::AssetTypeExtension;
        std::filesystem::path assetFullPath = destinationFolder / assetFilename;

        if (std::filesystem::exists(assetFullPath))
        {
            ATOM_WARNING("Texture {} already exists", assetFullPath.string());
            return AssetManager::GetUUIDForAssetPath(assetFullPath);
        }

        if (!std::filesystem::exists(destinationFolder))
            std::filesystem::create_directory(destinationFolder);

        s32 width, height;
        Vector<byte> decodedData;
        TextureFormat format = importSettings.Format;

        if (!DecodeImage(sourcePath, format, width, height, decodedData))
        {
            ATOM_ERROR("Failed creating texture asset {}", assetFullPath);
            return 0;
        }

        TextureDescription textureDesc;
        textureDesc.Format = format;
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = Utils::GetMaxMipCount(width, height);
        textureDesc.Filter = importSettings.Filter;
        textureDesc.Wrap = importSettings.Wrap;
        textureDesc.UsageFlags |= TextureBindFlags::UnorderedAccess;

        Vector<Vector<byte>> tex2DData;
        tex2DData.resize(textureDesc.MipLevels);
        tex2DData[0] = decodedData;

        if (importSettings.Type == TextureType::Texture2D)
        {
            Ref<Texture2D> asset = CreateRef<Texture2D>(textureDesc, tex2DData, importSettings.IsReadable, sourcePath.stem().string().c_str());
            Renderer::GenerateMips(asset);
            asset->m_MetaData.AssetFilepath = assetFullPath;
            asset->m_MetaData.SourceFilepath = sourcePath;

            if (!AssetSerializer::Serialize(asset))
            {
                ATOM_ERROR("Failed serializing texture asset {}", assetFullPath);
                return 0;
            }

            AssetManager::RegisterAsset(asset->m_MetaData);
            return asset->m_MetaData.UUID;
        }
        else if (importSettings.Type == TextureType::TextureCube)
        {
            Ref<Texture2D> tex2D = CreateRef<Texture2D>(textureDesc, tex2DData, importSettings.IsReadable, sourcePath.stem().string().c_str());
            Renderer::GenerateMips(tex2D);
            Ref<TextureCube> asset = Renderer::CreateEnvironmentMap(tex2D, importSettings.CubemapSize);
            asset->m_MetaData.AssetFilepath = assetFullPath;
            asset->m_MetaData.SourceFilepath = sourcePath;

            if (!AssetSerializer::Serialize(asset))
            {
                ATOM_ERROR("Failed serializing texture asset {}", assetFullPath);
                return 0;
            }

            AssetManager::RegisterAsset(asset->m_MetaData);
            return asset->m_MetaData.UUID;
        }

        return 0;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID ContentTools::ImportTextureAsset(const byte* compressedData, u32 dataSize, const String& assetName, const std::filesystem::path& destinationFolder, const TextureImportSettings& importSettings)
    {
        String assetFilename = assetName + Asset::AssetTypeExtension;
        std::filesystem::path assetFullPath = destinationFolder / assetFilename;

        if (std::filesystem::exists(assetFullPath))
        {
            ATOM_WARNING("Texture {} already exists", assetFullPath.string());
            return AssetManager::GetUUIDForAssetPath(assetFullPath);
        }

        if (!std::filesystem::exists(destinationFolder))
            std::filesystem::create_directory(destinationFolder);

        s32 width, height;
        Vector<byte> decodedData;
        TextureFormat format = importSettings.Format;

        if (!DecodeImage(compressedData, dataSize, format, width, height, decodedData))
        {
            ATOM_ERROR("Failed creating texture asset {}", assetFullPath);
            return 0;
        }

        TextureDescription textureDesc;
        textureDesc.Format = format;
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = Utils::GetMaxMipCount(width, height);
        textureDesc.Filter = importSettings.Filter;
        textureDesc.Wrap = importSettings.Wrap;
        textureDesc.UsageFlags |= TextureBindFlags::UnorderedAccess;

        Vector<Vector<byte>> tex2DData;
        tex2DData.resize(textureDesc.MipLevels);
        tex2DData[0] = decodedData;

        Ref<Texture2D> asset = CreateRef<Texture2D>(textureDesc, tex2DData, importSettings.IsReadable, assetName.c_str());
        Renderer::GenerateMips(asset);
        asset->m_MetaData.AssetFilepath = assetFullPath;

        if (!AssetSerializer::Serialize(asset))
        {
            ATOM_ERROR("Failed serializing texture asset {}", assetFullPath);
            return 0;
        }

        AssetManager::RegisterAsset(asset->m_MetaData);
        return asset->m_MetaData.UUID;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID ContentTools::ImportMeshAsset(const std::filesystem::path& sourcePath, const std::filesystem::path& destinationFolder, const MeshImportSettings& importSettings)
    {
        String assetFilename = sourcePath.stem().string() + Asset::AssetTypeExtension;
        std::filesystem::path assetFullPath = destinationFolder / assetFilename;

        if (std::filesystem::exists(assetFullPath))
        {
            ATOM_WARNING("Mesh {} already exists", assetFullPath.string());
            return AssetManager::GetUUIDForAssetPath(assetFullPath);
        }

        if (!std::filesystem::exists(destinationFolder))
            std::filesystem::create_directory(destinationFolder);

        u32 processingFlags = aiProcess_ImproveCacheLocality |
                              aiProcess_LimitBoneWeights |
                              aiProcess_RemoveRedundantMaterials |
                              aiProcess_SplitLargeMeshes |
                              aiProcess_Triangulate |
                              aiProcess_GenUVCoords |
                              aiProcess_CalcTangentSpace |
                              aiProcess_SortByPType |
                              aiProcess_FindDegenerates |
                              aiProcess_FindInvalidData |
                              aiProcess_FindInstances |
                              aiProcess_ValidateDataStructure |
                              aiProcess_OptimizeMeshes |
                              aiProcess_JoinIdenticalVertices |
                              aiProcess_ConvertToLeftHanded |
                              aiProcess_PreTransformVertices;
        
        if (importSettings.SmoothNormals)
            processingFlags |= aiProcess_GenSmoothNormals;
        if (importSettings.PreserveHierarchy)
            processingFlags &= ~aiProcess_PreTransformVertices;
        
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(sourcePath.string().c_str(), processingFlags);

        if (!scene)
        {
            ATOM_ERROR("Failed importing mesh file {}. Error: {}", sourcePath, importer.GetErrorString());
            return 0;
        }

        Vector<Vertex> vertices;
        vertices.reserve(5000);

        Vector<u32> indices;
        indices.reserve(10000);

        Vector<Submesh> submeshes;
        submeshes.reserve(scene->mNumMeshes);

        MaterialTable materialTable;

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
            Submesh& sm = submeshes.emplace_back();
            sm.StartVertex = startVertex;
            sm.VertexCount = vertexCount;
            sm.StartIndex = startIndex;
            sm.IndexCount = indexCount;
            sm.MaterialIndex = submesh->mMaterialIndex;
        }

        // Parse all materials
        for (u32 materialIdx = 0; materialIdx < scene->mNumMaterials; materialIdx++)
        {
            const aiMaterial* assimpMat = scene->mMaterials[materialIdx];

            // Set the name
            aiString materialName;
            assimpMat->Get(AI_MATKEY_NAME, materialName);

            UUID materialUUID = ContentTools::CreateMaterialAsset(materialName.C_Str(), "TestProject/Assets/Materials");
            Ref<Material> materialAsset = AssetManager::GetAsset<Material>(materialUUID, true);

            // Set albedo color
            aiColor4D albedo;
            if (assimpMat->Get(AI_MATKEY_COLOR_DIFFUSE, albedo) == AI_SUCCESS)
            {
                // Set transparency flag
                f32 opacity;
                if (assimpMat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS && opacity < 1.0f)
                {
                    materialAsset->SetFlag(MaterialFlags::Transparent, true);
                    albedo.a = opacity;
                }

                materialAsset->SetUniform("AlbedoColor", glm::vec4(albedo.r, albedo.g, albedo.b, albedo.a));
            }

            // Set roughness
            f32 roughness;
            if (assimpMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
            {
                materialAsset->SetUniform("Roughness", roughness);
            }

            // Set metalness
            f32 metalness;
            if (assimpMat->Get(AI_MATKEY_REFLECTIVITY, metalness) == AI_SUCCESS)
            {
                materialAsset->SetUniform("Metalness", metalness);
            }

            // Set two sided flag
            bool twoSided;
            if (assimpMat->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS && twoSided)
            {
                materialAsset->SetFlag(MaterialFlags::TwoSided, true);
            }

            // Set wireframe flag
            bool wireframe;
            if (assimpMat->Get(AI_MATKEY_ENABLE_WIREFRAME, wireframe) == AI_SUCCESS && wireframe)
            {
                materialAsset->SetFlag(MaterialFlags::Wireframe, true);
            }

            // Set textures
            auto SetMaterialTexture = [&](aiTextureType type, const char* uniformName)
            {
                aiString aiPath;
                if (assimpMat->GetTexture(type, 0, &aiPath) == AI_SUCCESS)
                {
                    TextureImportSettings importSettings;
                    importSettings.Format = type == aiTextureType_METALNESS || type == aiTextureType_SHININESS ? TextureFormat::R8 : TextureFormat::RGBA8;

                    UUID textureUUID = 0;
                    if (const aiTexture* aiTexture = scene->GetEmbeddedTexture(aiPath.C_Str()))
                    {
                        // Texture is embedded. Decode the data buffer.
                        std::filesystem::path textureName = std::filesystem::path(aiTexture->mFilename.C_Str()).stem();
                        textureUUID = ContentTools::ImportTextureAsset((byte*)aiTexture->pcData, aiTexture->mWidth, textureName.string(), "TestProject/Assets/Textures", importSettings);
                    }
                    else
                    {
                        // Load the texture from filepath
                        textureUUID = ContentTools::ImportTextureAsset(sourcePath.parent_path() / aiPath.C_Str(), "TestProject/Assets/Textures", importSettings);
                    }

                    materialAsset->SetTexture(uniformName, AssetManager::GetAsset<Texture2D>(textureUUID, true));
                    materialAsset->SetUniform(fmt::format("Use{}", uniformName).c_str(), 1);
                }
            };

            SetMaterialTexture(aiTextureType_DIFFUSE, "AlbedoMap");
            SetMaterialTexture(aiTextureType_NORMALS, "NormalMap");
            SetMaterialTexture(aiTextureType_METALNESS, "MetalnessMap");
            SetMaterialTexture(aiTextureType_SHININESS, "RoughnessMap");

            // Save the material
            if (!AssetSerializer::Serialize(materialAsset))
            {
                ATOM_ERROR("Failed serializing material {}", materialAsset->GetAssetFilepath().string());
                continue;
            }

            materialTable.SetMaterial(materialIdx, materialAsset);
        }

        // We have to make the mesh readable so that the data is available on the CPU during serialization
        Ref<Mesh> asset = CreateRef<Mesh>(vertices, indices, submeshes, materialTable, true);
        asset->m_MetaData.AssetFilepath = assetFullPath;
        asset->m_MetaData.SourceFilepath = sourcePath;
        asset->m_IsReadable = importSettings.IsReadable;

        if (!AssetSerializer::Serialize(asset))
        {
            ATOM_ERROR("Failed serializing mesh asset {}", assetFullPath);
            return 0;
        }

        AssetManager::RegisterAsset(asset->m_MetaData);
        return asset->m_MetaData.UUID;
      
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture2D> ContentTools::ImportTexture(const std::filesystem::path& sourcePath, const TextureImportSettings& importSettings)
    {
        s32 width, height;
        Vector<byte> decodedData;
        TextureFormat format = importSettings.Format;

        if (!DecodeImage(sourcePath, format, width, height, decodedData))
            return nullptr;

        TextureDescription textureDesc;
        textureDesc.Format = format;
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = Utils::GetMaxMipCount(width, height);
        textureDesc.Filter = importSettings.Filter;
        textureDesc.Wrap = importSettings.Wrap;
        textureDesc.UsageFlags |= TextureBindFlags::UnorderedAccess;

        Vector<Vector<byte>> tex2DData;
        tex2DData.resize(textureDesc.MipLevels);
        tex2DData[0] = decodedData;

        return CreateRef<Texture2D>(textureDesc, tex2DData, importSettings.IsReadable, sourcePath.stem().string().c_str());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<Texture2D> ContentTools::ImportTexture(const byte* compressedData, u32 dataSize, const String& name, const TextureImportSettings& importSettings)
    {
        s32 width, height;
        Vector<byte> decodedData;
        TextureFormat format = importSettings.Format;

        if (!DecodeImage(compressedData, dataSize, format, width, height, decodedData))
            return nullptr;

        TextureDescription textureDesc;
        textureDesc.Format = format;
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = Utils::GetMaxMipCount(width, height);
        textureDesc.Filter = importSettings.Filter;
        textureDesc.Wrap = importSettings.Wrap;
        textureDesc.UsageFlags |= TextureBindFlags::UnorderedAccess;

        Vector<Vector<byte>> tex2DData;
        tex2DData.resize(textureDesc.MipLevels);
        tex2DData[0] = decodedData;

        return CreateRef<Texture2D>(textureDesc, tex2DData, importSettings.IsReadable, name.c_str());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID ContentTools::CreateMaterialAsset(const String& assetName, const std::filesystem::path& destinationFolder)
    {
        String assetFilename = assetName + Asset::AssetTypeExtension;
        std::filesystem::path assetFullPath = destinationFolder / assetFilename;

        if (std::filesystem::exists(assetFullPath))
        {
            ATOM_WARNING("Material {} already exists", assetFullPath.string());
            return AssetManager::GetUUIDForAssetPath(assetFullPath);
        }

        if (!std::filesystem::exists(destinationFolder))
            std::filesystem::create_directory(destinationFolder);

        Ref<Material> asset = CreateRef<Material>(Renderer::GetShaderLibrary().Get<GraphicsShader>("MeshPBRShader"), MaterialFlags::DepthTested);
        asset->m_MetaData.AssetFilepath = assetFullPath;

        if (!AssetSerializer::Serialize(asset))
        {
            ATOM_ERROR("Failed serializing material asset {}", assetFullPath);
            return 0;
        }

        AssetManager::RegisterAsset(asset->m_MetaData);
        return asset->m_MetaData.UUID;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool ContentTools::DecodeImage(const std::filesystem::path& sourcePath, TextureFormat& format, s32& width, s32& height, Vector<byte>& pixels)
    {
        String sourcePathStr = sourcePath.string();
        bool isHDR = stbi_is_hdr(sourcePathStr.c_str());

        if (isHDR && !Utils::IsHDRFormat(format))
        {
            ATOM_WARNING("Specified texture format is not compatible with HDR textures. Adjusting to the corresponding HDR format.");
            format = Utils::GetHDRFormatFromRegularFormat(format);
        }

        s32 desiredChannels = Utils::GetDesiredChannelsFromFormat(format);
        if (desiredChannels == 0)
        {
            ATOM_ERROR("Failed importing texture file {}. Invalid texture format was specified.", sourcePath);
            return false;
        }

        byte* data = isHDR ? (byte*)stbi_loadf(sourcePathStr.c_str(), &width, &height, nullptr, desiredChannels) :
                                    stbi_load(sourcePathStr.c_str(), &width, &height, nullptr, desiredChannels);
        if (!data)
        {
            ATOM_ERROR("Failed importing texture file {}", sourcePath);
            return false;
        }

        pixels.assign(data, data + width * height * Utils::GetFormatByteSize(format));
        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool ContentTools::DecodeImage(const byte* compressedData, u32 dataSize, TextureFormat& format, s32& width, s32& height, Vector<byte>& pixels)
    {
        bool isHDR = stbi_is_hdr_from_memory(compressedData, dataSize);

        if (isHDR && !Utils::IsHDRFormat(format))
        {
            ATOM_WARNING("Specified texture format is not compatible with HDR textures. Adjusting to the corresponding HDR format.");
            format = Utils::GetHDRFormatFromRegularFormat(format);
        }

        s32 desiredChannels = Utils::GetDesiredChannelsFromFormat(format);
        if (desiredChannels == 0)
        {
            ATOM_ERROR("Failed importing texture from compressed data. Invalid texture format was specified.");
            return false;
        }

        byte* decompressedData = isHDR ? (byte*)stbi_loadf_from_memory(compressedData, dataSize, &width, &height, nullptr, desiredChannels) :
                                                stbi_load_from_memory(compressedData, dataSize, &width, &height, nullptr, desiredChannels);
        if (!decompressedData)
        {
            ATOM_ERROR("Failed importing texture from compressed data. Decimpressing the data failed.");
            return false;
        }

        pixels.assign(decompressedData, decompressedData + width * height * Utils::GetFormatByteSize(format));
        return true;
    }
}
