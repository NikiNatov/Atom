#include "atompch.h"
#include "ContentTools.h"

#include "Atom/Asset/AssetSerializer.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/Mesh.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Scene/Scene.h"

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

        static glm::mat4 AssimpMat4ToGLM(const aiMatrix4x4& matrix)
        {
            glm::mat4 result;

            result[0].x = matrix.a1; result[0].y = matrix.b1; result[0].z = matrix.c1; result[0].w = matrix.d1;
            result[1].x = matrix.a2; result[1].y = matrix.b2; result[1].z = matrix.c2; result[1].w = matrix.d2;
            result[2].x = matrix.a3; result[2].y = matrix.b3; result[2].z = matrix.c3; result[2].w = matrix.d3;
            result[3].x = matrix.a4; result[3].y = matrix.b4; result[3].z = matrix.c4; result[3].w = matrix.d4;

            return result;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID ContentTools::ImportTextureAsset(const std::filesystem::path& sourcePath, const std::filesystem::path& destinationFolder, const TextureImportSettings& importSettings)
    {
        const char* extension = Asset::AssetFileExtensions[importSettings.Type == TextureType::Texture2D ? (u32)AssetType::Texture2D : (u32)AssetType::TextureCube];
        String assetFilename = sourcePath.stem().string() + extension;
        std::filesystem::path assetFullPath = AssetManager::GetAssetFullPath(destinationFolder / assetFilename);

        if (std::filesystem::exists(assetFullPath))
        {
            ATOM_WARNING("Texture {} already exists", assetFullPath.string());
            return AssetManager::GetUUIDForAssetPath(assetFullPath);
        }

        if (!std::filesystem::exists(assetFullPath.parent_path()))
            std::filesystem::create_directories(assetFullPath.parent_path());

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
        textureDesc.MipLevels = Texture::CalculateMaxMipCount(width, height);
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
            asset->m_MetaData.SourceFilepath = sourcePath;

            if (!AssetSerializer::Serialize(assetFullPath, asset))
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
            asset->m_MetaData.SourceFilepath = sourcePath;

            if (!AssetSerializer::Serialize(assetFullPath, asset))
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
        const char* extension = Asset::AssetFileExtensions[importSettings.Type == TextureType::Texture2D ? (u32)AssetType::Texture2D : (u32)AssetType::TextureCube];
        String assetFilename = assetName + extension;
        std::filesystem::path assetFullPath = AssetManager::GetAssetFullPath(destinationFolder / assetFilename);

        if (std::filesystem::exists(assetFullPath))
        {
            ATOM_WARNING("Texture {} already exists", assetFullPath.string());
            return AssetManager::GetUUIDForAssetPath(assetFullPath);
        }

        if (!std::filesystem::exists(assetFullPath.parent_path()))
            std::filesystem::create_directories(assetFullPath.parent_path());

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
        textureDesc.MipLevels = Texture::CalculateMaxMipCount(width, height);
        textureDesc.Filter = importSettings.Filter;
        textureDesc.Wrap = importSettings.Wrap;
        textureDesc.UsageFlags |= TextureBindFlags::UnorderedAccess;

        Vector<Vector<byte>> tex2DData;
        tex2DData.resize(textureDesc.MipLevels);
        tex2DData[0] = decodedData;

        Ref<Texture2D> asset = CreateRef<Texture2D>(textureDesc, tex2DData, importSettings.IsReadable, assetName.c_str());
        Renderer::GenerateMips(asset);

        if (!AssetSerializer::Serialize(assetFullPath, asset))
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
        String assetFilename = sourcePath.stem().string() + Asset::AssetFileExtensions[(u32)AssetType::Mesh];
        std::filesystem::path assetFullPath = AssetManager::GetAssetFullPath(destinationFolder / assetFilename);

        if (std::filesystem::exists(assetFullPath))
        {
            ATOM_WARNING("Mesh {} already exists", assetFullPath.string());
            return AssetManager::GetUUIDForAssetPath(assetFullPath);
        }

        if (!std::filesystem::exists(assetFullPath.parent_path()))
            std::filesystem::create_directories(assetFullPath.parent_path());

        u32 processingFlags = aiProcess_ImproveCacheLocality |
                              aiProcess_LimitBoneWeights |
                              aiProcess_RemoveRedundantMaterials |
                              aiProcess_SplitLargeMeshes |
                              aiProcess_Triangulate |
                              aiProcess_GenUVCoords |
                              aiProcess_CalcTangentSpace |
                              aiProcess_SortByPType |
                              aiProcess_FindDegenerates |
                              aiProcess_FindInstances |
                              aiProcess_ValidateDataStructure |
                              aiProcess_OptimizeMeshes |
                              aiProcess_JoinIdenticalVertices |
                              aiProcess_ConvertToLeftHanded |
                              aiProcess_PreTransformVertices;
        
        if (importSettings.SmoothNormals)
            processingFlags |= aiProcess_GenSmoothNormals;
        if (importSettings.ImportAnimations)
        {
            processingFlags &= ~aiProcess_PreTransformVertices;
            processingFlags |= aiProcess_OptimizeGraph;
        }
        
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(sourcePath.string().c_str(), processingFlags);

        if (!scene)
        {
            ATOM_ERROR("Failed importing mesh file {}. Error: {}", sourcePath, importer.GetErrorString());
            return 0;
        }

        MeshDescription meshDesc;
        meshDesc.Positions.reserve(5000);
        meshDesc.UVs.reserve(5000);
        meshDesc.Normals.reserve(5000);
        meshDesc.Tangents.reserve(5000);
        meshDesc.Bitangents.reserve(5000);
        meshDesc.Indices.reserve(10000);
        meshDesc.Submeshes.reserve(scene->mNumMeshes);
        meshDesc.MaterialTable = CreateRef<MaterialTable>();

        // Parse all submeshes
        HashMap<String, std::pair<u32, glm::mat4>> bonesByName;

        for (u32 submeshIdx = 0; submeshIdx < scene->mNumMeshes; submeshIdx++)
        {
            aiMesh* submesh = scene->mMeshes[submeshIdx];

            u32 startVertex = meshDesc.Positions.size();
            u32 startIndex = meshDesc.Indices.size();

            // Construct all vertices
            u32 vertexCount = 0;
            for (u32 vertexIdx = 0; vertexIdx < submesh->mNumVertices; vertexIdx++)
            {
                const aiVector3D& position = submesh->mVertices[vertexIdx];
                const aiVector3D& texCoord = submesh->mTextureCoords[0][vertexIdx];
                const aiVector3D& normal = submesh->mNormals[vertexIdx];
                const aiVector3D& tangent = submesh->mTangents[vertexIdx];
                const aiVector3D& bitangent = submesh->mBitangents[vertexIdx];

                meshDesc.Positions.emplace_back(position.x, position.y, position.z);
                meshDesc.UVs.emplace_back(texCoord.x, texCoord.y);
                meshDesc.Normals.emplace_back(normal.x, normal.y, normal.z);
                meshDesc.Tangents.emplace_back(tangent.x, tangent.y, tangent.z);
                meshDesc.Bitangents.emplace_back(bitangent.x, bitangent.y, bitangent.z);
                vertexCount++;
            }

            // Construct all indices
            u32 indexCount = 0;
            for (u32 faceIdx = 0; faceIdx < submesh->mNumFaces; faceIdx++)
            {
                const aiFace& face = submesh->mFaces[faceIdx];
                for (u32 i = 0; i < face.mNumIndices; i++)
                {
                    meshDesc.Indices.push_back(face.mIndices[i]);
                    indexCount++;
                }
            }

            // Parse bone data
            meshDesc.BoneWeights.resize(startVertex + vertexCount);
            Vector<u32> boneWeightsPerVertex(vertexCount);

            for (u32 boneIdx = 0; boneIdx < submesh->mNumBones; boneIdx++)
            {
                aiBone* bone = submesh->mBones[boneIdx];
                String boneName = bone->mName.C_Str();

                if (bonesByName.find(boneName) == bonesByName.end())
                    bonesByName[boneName] = { bonesByName.size(), Utils::AssimpMat4ToGLM(bone->mOffsetMatrix) };

                for (u32 weightIdx = 0; weightIdx < bone->mNumWeights; weightIdx++)
                {
                    const aiVertexWeight& vertexWeight = bone->mWeights[weightIdx];
                    u32 currentWeightIdx = boneWeightsPerVertex[vertexWeight.mVertexId]++;
                    ATOM_ENGINE_ASSERT(currentWeightIdx < Skeleton::Bone::MAX_BONE_WEIGHTS);
                    meshDesc.BoneWeights[startVertex + vertexWeight.mVertexId].Weights[currentWeightIdx] = { bonesByName.at(boneName).first, vertexWeight.mWeight };
                }
            }

            // Create submesh
            Submesh& sm = meshDesc.Submeshes.emplace_back();
            sm.StartVertex = startVertex;
            sm.VertexCount = vertexCount;
            sm.StartIndex = startIndex;
            sm.IndexCount = indexCount;
            sm.MaterialIndex = submesh->mMaterialIndex;
        }

        if (importSettings.ImportAnimations)
        {
            // Create skeleton
            Vector<Skeleton::Bone> skeletonBones;
            skeletonBones.resize(bonesByName.size());

            Queue<aiNode*> nodeQueue;
            nodeQueue.push(scene->mRootNode);

            while (!nodeQueue.empty())
            {
                aiNode* currentNode = nodeQueue.front();
                String nodeName = currentNode->mName.C_Str();

                if (bonesByName.find(nodeName) != bonesByName.end())
                {
                    // Process the node if it is a bone
                    auto& [id, inverseBindTransform] = bonesByName[nodeName];

                    Skeleton::Bone& bone = skeletonBones[id];
                    bone.ID = id;
                    bone.InverseBindTransform = inverseBindTransform;

                    // Find the bone parent
                    aiNode* currentParent = currentNode->mParent;
                    while (currentParent)
                    {
                        if (bonesByName.find(currentParent->mName.C_Str()) != bonesByName.end())
                            break;

                        currentParent = currentParent->mParent;
                    }

                    if (!currentParent)
                    {
                        // We found a root node
                        bone.ParentID = UINT32_MAX;
                    }
                    else
                    {
                        u32 parentID = bonesByName.at(currentParent->mName.C_Str()).first;
                        bone.ParentID = parentID;
                        skeletonBones[parentID].ChildrenIDs.push_back(id);
                    }
                }

                for (u32 childIdx = 0; childIdx < currentNode->mNumChildren; childIdx++)
                    nodeQueue.push(currentNode->mChildren[childIdx]);

                nodeQueue.pop();
            }

            String skeletonName = sourcePath.stem().string() + "_Skeleton" + Asset::AssetFileExtensions[(u32)AssetType::Skeleton];
            ContentTools::CreateSkeletonAsset(skeletonBones, std::filesystem::path("Skeletons") / skeletonName);

            // Parse all animations
            for (u32 animationIdx = 0; animationIdx < scene->mNumAnimations; animationIdx++)
            {
                aiAnimation* animation = scene->mAnimations[animationIdx];
                HashMap<f32, HashMap<u32, Animation::BoneTransform>> boneTransformsPerKeyFrame;

                for (u32 nodeIdx = 0; nodeIdx < animation->mNumChannels; nodeIdx++)
                {
                    aiNodeAnim* node = animation->mChannels[nodeIdx];
                    String boneName = node->mNodeName.C_Str();

                    if (bonesByName.find(boneName) != bonesByName.end())
                    {
                        u32 boneID = bonesByName.at(boneName).first;
                    
                        for (u32 keyIdx = 0; keyIdx < node->mNumPositionKeys; keyIdx++)
                        {
                            const aiVectorKey& posKey = node->mPositionKeys[keyIdx];
                            f32 timeStamp = posKey.mTime;
                            boneTransformsPerKeyFrame[timeStamp][boneID].Position = glm::vec3(posKey.mValue.x, posKey.mValue.y, posKey.mValue.z);
                        }
                    
                        for (u32 keyIdx = 0; keyIdx < node->mNumRotationKeys; keyIdx++)
                        {
                            const aiQuatKey& rotKey = node->mRotationKeys[keyIdx];
                            f32 timeStamp = rotKey.mTime;
                            boneTransformsPerKeyFrame[timeStamp][boneID].Rotation = glm::quat(rotKey.mValue.w, rotKey.mValue.x, rotKey.mValue.y, rotKey.mValue.z);
                        }
                    
                        for (u32 keyIdx = 0; keyIdx < node->mNumScalingKeys; keyIdx++)
                        {
                            const aiVectorKey& scaleKey = node->mScalingKeys[keyIdx];
                            f32 timeStamp = scaleKey.mTime;
                            boneTransformsPerKeyFrame[timeStamp][boneID].Scale = glm::vec3(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z);
                        }
                    }
                }

                // Construct key frames
                Set<Animation::KeyFrame> keyFrames;
                for (auto& [timeStamp, boneTransforms] : boneTransformsPerKeyFrame)
                    keyFrames.insert({ timeStamp, boneTransforms });

                String animationName = animation->mName.C_Str();
                animationName = animationName.substr(animationName.find_last_of('|') + 1);
                animationName = fmt::format("{}_{}{}", sourcePath.stem().string(), animationName, Asset::AssetFileExtensions[(u32)AssetType::Animation]);

                ContentTools::CreateAnimationAsset(animation->mDuration, animation->mTicksPerSecond, keyFrames, std::filesystem::path("Animations") / animationName);
            }
        }

        // Parse all materials
        for (u32 materialIdx = 0; materialIdx < scene->mNumMaterials; materialIdx++)
        {
            const aiMaterial* assimpMat = scene->mMaterials[materialIdx];

            // Set the name
            aiString materialName;
            assimpMat->Get(AI_MATKEY_NAME, materialName);
            materialName.Append(Asset::AssetFileExtensions[(u32)AssetType::Material]);

            UUID materialUUID = ContentTools::CreateMaterialAsset(std::filesystem::path("Materials") / materialName.C_Str());
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
                        textureUUID = ContentTools::ImportTextureAsset((byte*)aiTexture->pcData, aiTexture->mWidth, textureName.string(), "Textures", importSettings);
                    }
                    else
                    {
                        // Load the texture from filepath
                        textureUUID = ContentTools::ImportTextureAsset(sourcePath.parent_path() / aiPath.C_Str(), "Textures", importSettings);
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
            if (!AssetSerializer::Serialize(materialAsset->GetAssetFilepath(), materialAsset))
            {
                ATOM_ERROR("Failed serializing material {}", materialAsset->GetAssetFilepath().string());
                continue;
            }

            meshDesc.MaterialTable->SetMaterial(materialIdx, materialAsset);
        }

        // We have to make the mesh readable so that the data is available on the CPU during serialization
        Ref<Mesh> asset = CreateRef<Mesh>(meshDesc, true);
        asset->m_MetaData.SourceFilepath = sourcePath;
        asset->m_IsReadable = importSettings.IsReadable;

        if (!AssetSerializer::Serialize(assetFullPath, asset))
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
        textureDesc.MipLevels = Texture::CalculateMaxMipCount(width, height);
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
        textureDesc.MipLevels = Texture::CalculateMaxMipCount(width, height);
        textureDesc.Filter = importSettings.Filter;
        textureDesc.Wrap = importSettings.Wrap;
        textureDesc.UsageFlags |= TextureBindFlags::UnorderedAccess;

        Vector<Vector<byte>> tex2DData;
        tex2DData.resize(textureDesc.MipLevels);
        tex2DData[0] = decodedData;

        return CreateRef<Texture2D>(textureDesc, tex2DData, importSettings.IsReadable, name.c_str());
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID ContentTools::CreateAnimationAsset(f32 duration, f32 ticksPerSecond, const Set<Animation::KeyFrame>& keyFrames, const std::filesystem::path& filepath)
    {
        std::filesystem::path assetFullPath = AssetManager::GetAssetFullPath(filepath);

        if (std::filesystem::exists(assetFullPath))
        {
            ATOM_WARNING("Animation {} already exists", assetFullPath.string());
            return AssetManager::GetUUIDForAssetPath(assetFullPath);
        }

        if (!std::filesystem::exists(assetFullPath.parent_path()))
            std::filesystem::create_directories(assetFullPath.parent_path());

        Ref<Animation> asset = CreateRef<Animation>(duration, ticksPerSecond, keyFrames);

        if (!AssetSerializer::Serialize(assetFullPath, asset))
        {
            ATOM_ERROR("Failed serializing animation asset {}", assetFullPath);
            return 0;
        }

        AssetManager::RegisterAsset(asset->m_MetaData);
        return asset->m_MetaData.UUID;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID ContentTools::CreateSkeletonAsset(const Vector<Skeleton::Bone>& bones, const std::filesystem::path& filepath)
    {
        std::filesystem::path assetFullPath = AssetManager::GetAssetFullPath(filepath);

        if (std::filesystem::exists(assetFullPath))
        {
            ATOM_WARNING("Skeleton {} already exists", assetFullPath.string());
            return AssetManager::GetUUIDForAssetPath(assetFullPath);
        }

        if (!std::filesystem::exists(assetFullPath.parent_path()))
            std::filesystem::create_directories(assetFullPath.parent_path());

        Ref<Skeleton> asset = CreateRef<Skeleton>(bones);

        if (!AssetSerializer::Serialize(assetFullPath, asset))
        {
            ATOM_ERROR("Failed serializing skeleton asset {}", assetFullPath);
            return 0;
        }

        AssetManager::RegisterAsset(asset->m_MetaData);
        return asset->m_MetaData.UUID;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID ContentTools::CreateMaterialAsset(const std::filesystem::path& filepath)
    {
        Ref<Material> asset = CreateRef<Material>(Renderer::GetShaderLibrary().Get<GraphicsShader>("MeshPBRAnimatedShader"), MaterialFlags::DepthTested);
        std::filesystem::path assetFullPath = AssetManager::GetAssetFullPath(filepath);

        if (!filepath.empty())
        {
            if (std::filesystem::exists(filepath))
            {
                ATOM_WARNING("Material {} already exists", filepath.string());
                return AssetManager::GetUUIDForAssetPath(filepath);
            }

            if (!std::filesystem::exists(assetFullPath.parent_path()))
                std::filesystem::create_directories(assetFullPath.parent_path());

            if (!AssetSerializer::Serialize(assetFullPath, asset))
            {
                ATOM_ERROR("Failed serializing material asset {}", filepath);
                return 0;
            }

            AssetManager::RegisterAsset(asset->m_MetaData);
        }
        else
        {
            AssetManager::RegisterAsset(asset);
        }

        return asset->m_MetaData.UUID;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    UUID ContentTools::CreateSceneAsset(const String& sceneName, const std::filesystem::path& filepath)
    {
        Ref<Scene> asset = CreateRef<Scene>(sceneName);
        std::filesystem::path assetFullPath = AssetManager::GetAssetFullPath(filepath);

        if (!filepath.empty())
        {
            if (std::filesystem::exists(filepath))
            {
                ATOM_WARNING("Scene {} already exists", filepath.string());
                return AssetManager::GetUUIDForAssetPath(filepath);
            }

            if (!std::filesystem::exists(assetFullPath.parent_path()))
                std::filesystem::create_directories(assetFullPath.parent_path());

            if (!AssetSerializer::Serialize(assetFullPath, asset))
            {
                ATOM_ERROR("Failed serializing scene asset {}", filepath);
                return 0;
            }

            AssetManager::RegisterAsset(asset->m_MetaData);
        }
        else
        {
            AssetManager::RegisterAsset(asset);
        }

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
        stbi_image_free(data);
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
        stbi_image_free(decompressedData);
        return true;
    }
}
