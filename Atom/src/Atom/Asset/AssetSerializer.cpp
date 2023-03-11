#include "atompch.h"
#include "AssetSerializer.h"

#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/Mesh.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/Animation.h"
#include "Atom/Scene/Scene.h"
#include "Atom/Scene/Components.h"
#include "Atom/Scripting/ScriptEngine.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    static bool AssetSerializer::Serialize(const std::filesystem::path& filepath, Ref<Texture2D> asset)
    {
        std::ofstream ofs(filepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        std::filesystem::path absolutePath = std::filesystem::canonical(filepath);

        if (asset->GetAssetFlag(AssetFlags::Serialized) && asset->m_MetaData.AssetFilepath != absolutePath)
        {
            // If the asset was already serialized but the path is different than the one passed as a paraeter, create a copy of the asset with a new ID
            AssetMetaData newMetaData = asset->m_MetaData;
            newMetaData.UUID = UUID();
            newMetaData.AssetFilepath = absolutePath;
            SerializeMetaData(ofs, newMetaData);
        }
        else
        {
            asset->m_MetaData.AssetFilepath = absolutePath;
            asset->SetAssetFlag(AssetFlags::Serialized);
            SerializeMetaData(ofs, asset->m_MetaData);
        }

        u32 pixelDataSize = asset->m_PixelData.size();
        ofs.write((char*)&asset->m_Description.Format, sizeof(TextureFormat));
        ofs.write((char*)&asset->m_Description.Width, sizeof(u32));
        ofs.write((char*)&asset->m_Description.Height, sizeof(u32));
        ofs.write((char*)&asset->m_Description.MipLevels, sizeof(u32));
        ofs.write((char*)&asset->m_Description.Filter, sizeof(TextureFilter));
        ofs.write((char*)&asset->m_Description.Wrap, sizeof(TextureWrap));
        ofs.write((char*)&asset->m_Description.UsageFlags, sizeof(TextureBindFlags));
        ofs.write((char*)&asset->m_IsReadable, sizeof(bool));

        if (asset->m_IsReadable)
        {
            for (u32 mip = 0; mip < asset->m_Description.MipLevels; mip++)
            {
                u32 pixelDataSize = asset->m_PixelData[mip].size();
                ofs.write((char*)&pixelDataSize, sizeof(u32));
                ofs.write((char*)asset->m_PixelData[mip].data(), pixelDataSize);
            }
        }
        else
        {
            for (u32 mip = 0; mip < asset->m_Description.MipLevels; mip++)
            {
                Ref<ReadbackBuffer> buffer = Renderer::ReadbackTextureData(asset.get(), mip);
                void* mappedData = buffer->Map(0, 0);

                u32 pixelDataSize = buffer->GetSize();
                ofs.write((char*)&pixelDataSize, sizeof(u32));
                ofs.write((char*)mappedData, pixelDataSize);

                buffer->Unmap();
            }
        }

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    static bool AssetSerializer::Serialize(const std::filesystem::path& filepath, Ref<TextureCube> asset)
    {
        std::ofstream ofs(filepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        std::filesystem::path absolutePath = std::filesystem::canonical(filepath);

        if (asset->GetAssetFlag(AssetFlags::Serialized) && asset->m_MetaData.AssetFilepath != absolutePath)
        {
            // If the asset was already serialized but the path is different than the one passed as a paraeter, create a copy of the asset with a new ID
            AssetMetaData newMetaData = asset->m_MetaData;
            newMetaData.UUID = UUID();
            newMetaData.AssetFilepath = absolutePath;
            SerializeMetaData(ofs, newMetaData);
        }
        else
        {
            asset->m_MetaData.AssetFilepath = absolutePath;
            asset->SetAssetFlag(AssetFlags::Serialized);
            SerializeMetaData(ofs, asset->m_MetaData);
        }

        ofs.write((char*)&asset->m_Description.Format, sizeof(TextureFormat));
        ofs.write((char*)&asset->m_Description.Width, sizeof(u32));
        ofs.write((char*)&asset->m_Description.Height, sizeof(u32));
        ofs.write((char*)&asset->m_Description.MipLevels, sizeof(u32));
        ofs.write((char*)&asset->m_Description.Filter, sizeof(TextureFilter));
        ofs.write((char*)&asset->m_Description.Wrap, sizeof(TextureWrap));
        ofs.write((char*)&asset->m_Description.UsageFlags, sizeof(TextureBindFlags));
        ofs.write((char*)&asset->m_IsReadable, sizeof(bool));

        if (asset->m_IsReadable)
        {
            for (u32 face = 0; face < 6; face++)
            {
                for (u32 mip = 0; mip < asset->m_Description.MipLevels; mip++)
                {
                    u32 pixelDataSize = asset->m_PixelData[face][mip].size();
                    ofs.write((char*)&pixelDataSize, sizeof(u32));
                    ofs.write((char*)asset->m_PixelData[face][mip].data(), pixelDataSize);
                }
            }
        }
        else
        {
            for (u32 face = 0; face < 6; face++)
            {
                for (u32 mip = 0; mip < asset->m_Description.MipLevels; mip++)
                {
                    Ref<ReadbackBuffer> buffer = Renderer::ReadbackTextureData(asset.get(), mip, face);
                    void* mappedData = buffer->Map(0, 0);

                    u32 pixelDataSize = buffer->GetSize();
                    ofs.write((char*)&pixelDataSize, sizeof(u32));
                    ofs.write((char*)mappedData, pixelDataSize);

                    buffer->Unmap();
                }
            }
        }

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    static bool AssetSerializer::Serialize(const std::filesystem::path& filepath, Ref<Material> asset)
    {
        std::ofstream ofs(filepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        std::filesystem::path absolutePath = std::filesystem::canonical(filepath);

        if (asset->GetAssetFlag(AssetFlags::Serialized) && asset->m_MetaData.AssetFilepath != absolutePath)
        {
            // If the asset was already serialized but the path is different than the one passed as a paraeter, create a copy of the asset with a new ID
            AssetMetaData newMetaData = asset->m_MetaData;
            newMetaData.UUID = UUID();
            newMetaData.AssetFilepath = absolutePath;
            SerializeMetaData(ofs, newMetaData);
        }
        else
        {
            asset->m_MetaData.AssetFilepath = absolutePath;
            asset->SetAssetFlag(AssetFlags::Serialized);
            SerializeMetaData(ofs, asset->m_MetaData);
        }

        // Serialize shader name
        u32 shaderNameLength = asset->m_Shader->GetName().length();
        ofs.write((char*)&shaderNameLength, sizeof(u32));
        ofs.write(asset->m_Shader->GetName().data(), shaderNameLength);

        // Serialize flags
        MaterialFlags flags = asset->GetFlags();
        ofs.write((char*)&flags, sizeof(MaterialFlags));

        // Serialize uniform buffers
        u32 uniformBufferCount = asset->GetUniformBuffersData().size();
        ofs.write((char*)&uniformBufferCount, sizeof(u32));

        for (auto& [bufferRegister, bufferData] : asset->GetUniformBuffersData())
        {
            u32 bufferSize = bufferData.size();
            ofs.write((char*)&bufferRegister, sizeof(u32));
            ofs.write((char*)&bufferSize, sizeof(u32));
            ofs.write((char*)bufferData.data(), bufferSize);
        }

        // Serialize textures
        u32 textureCount = asset->GetTextures().size();
        ofs.write((char*)&textureCount, sizeof(u32));

        for (auto& [textureRegister, texture] : asset->GetTextures())
        {
            UUID textureHandle = 0;

            if (texture)
            {
                switch (texture->GetType())
                {
                    case TextureType::Texture2D:
                    {
                        Ref<Texture2D> texture = std::dynamic_pointer_cast<Texture2D>(asset->m_Textures[textureRegister]);
                        ATOM_ENGINE_ASSERT(texture);
                        textureHandle = texture->GetUUID();
                        break;
                    }
                    case TextureType::TextureCube:
                    {
                        Ref<TextureCube> texture = std::dynamic_pointer_cast<TextureCube>(asset->m_Textures[textureRegister]);
                        ATOM_ENGINE_ASSERT(texture);
                        textureHandle = texture->GetUUID();
                        break;
                    }
                }
            }

            ofs.write((char*)&textureRegister, sizeof(u32));
            ofs.write((char*)&textureHandle, sizeof(u64));
        }

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    static bool AssetSerializer::Serialize(const std::filesystem::path& filepath, Ref<Mesh> asset)
    {
        std::ofstream ofs(filepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        std::filesystem::path absolutePath = std::filesystem::canonical(filepath);

        if (asset->GetAssetFlag(AssetFlags::Serialized) && asset->m_MetaData.AssetFilepath != absolutePath)
        {
            // If the asset was already serialized but the path is different than the one passed as a paraeter, create a copy of the asset with a new ID
            AssetMetaData newMetaData = asset->m_MetaData;
            newMetaData.UUID = UUID();
            newMetaData.AssetFilepath = absolutePath;
            SerializeMetaData(ofs, newMetaData);
        }
        else
        {
            asset->m_MetaData.AssetFilepath = absolutePath;
            asset->SetAssetFlag(AssetFlags::Serialized);
            SerializeMetaData(ofs, asset->m_MetaData);
        }

        u32 vertexCount = asset->m_Positions.size();
        ofs.write((char*)&vertexCount, sizeof(u32));
        ofs.write((char*)asset->m_Positions.data(), sizeof(glm::vec3) * vertexCount);
        ofs.write((char*)asset->m_UVs.data(), sizeof(glm::vec2) * vertexCount);
        ofs.write((char*)asset->m_Normals.data(), sizeof(glm::vec3) * vertexCount);
        ofs.write((char*)asset->m_Tangents.data(), sizeof(glm::vec3) * vertexCount);
        ofs.write((char*)asset->m_Bitangents.data(), sizeof(glm::vec3) * vertexCount);
        ofs.write((char*)asset->m_BoneWeights.data(), sizeof(BoneWeight) * vertexCount);

        u32 indexCount = asset->m_Indices.size();
        ofs.write((char*)&indexCount, sizeof(u32));
        ofs.write((char*)asset->m_Indices.data(), sizeof(u32) * indexCount);

        u32 submeshCount = asset->m_Submeshes.size();
        ofs.write((char*)&submeshCount, sizeof(u32));
        ofs.write((char*)asset->m_Submeshes.data(), sizeof(Submesh) * submeshCount);

        ofs.write((char*)&asset->m_IsReadable, sizeof(bool));

        for (u32 submeshIdx = 0; submeshIdx < submeshCount; submeshIdx++)
        {
            UUID materialUUID = 0;

            if (Ref<Material> material = asset->m_MaterialTable->GetMaterial(submeshIdx))
                materialUUID = material->m_MetaData.UUID;

            ofs.write((char*)&materialUUID, sizeof(u64));
        }

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    static bool AssetSerializer::Serialize(const std::filesystem::path& filepath, Ref<Scene> asset)
    {
        std::ofstream ofs(filepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        std::filesystem::path absolutePath = std::filesystem::canonical(filepath);

        if (asset->GetAssetFlag(AssetFlags::Serialized) && asset->m_MetaData.AssetFilepath != absolutePath)
        {
            // If the asset was already serialized but the path is different than the one passed as a paraeter, create a copy of the asset with a new ID
            AssetMetaData newMetaData = asset->m_MetaData;
            newMetaData.UUID = UUID();
            newMetaData.AssetFilepath = absolutePath;
            SerializeMetaData(ofs, newMetaData);
        }
        else
        {
            asset->m_MetaData.AssetFilepath = absolutePath;
            asset->SetAssetFlag(AssetFlags::Serialized);
            SerializeMetaData(ofs, asset->m_MetaData);
        }
        
        u32 nameSize = asset->m_Name.size();
        ofs.write((char*)&nameSize, sizeof(u32));
        ofs.write((char*)asset->m_Name.data(), nameSize);

        u32 entityCount = asset->m_Registry.alive();
        ofs.write((char*)&entityCount, sizeof(u32));

        asset->m_Registry.each([&](auto entityID)
        {
            Entity entity = { entityID, asset.get() };

            bool isValid = (bool)entity;
            ofs.write((char*)&isValid, sizeof(bool));

            if (!isValid)
                return;

            UUID uuid = entity.GetUUID();
            ofs.write((char*)&uuid, sizeof(UUID));

            bool hasTagComponent = entity.HasComponent<TagComponent>();
            ofs.write((char*)&hasTagComponent, sizeof(bool));

            if (hasTagComponent)
            {
                auto& tc = entity.GetComponent<TagComponent>();
                u32 tagSize = tc.Tag.size();
                ofs.write((char*)&tagSize, sizeof(u32));
                ofs.write((char*)tc.Tag.data(), tagSize);
            }

            bool hasSceneHierarchyComponent = entity.HasComponent<SceneHierarchyComponent>();
            ofs.write((char*)&hasSceneHierarchyComponent, sizeof(bool));

            if (hasSceneHierarchyComponent)
            {
                auto& shc = entity.GetComponent<SceneHierarchyComponent>();
                ofs.write((char*)&shc, sizeof(SceneHierarchyComponent));
            }

            bool hasTransformComponent = entity.HasComponent<TransformComponent>();
            ofs.write((char*)&hasTransformComponent, sizeof(bool));

            if (hasTransformComponent)
            {
                auto& tc = entity.GetComponent<TransformComponent>();
                ofs.write((char*)&tc, sizeof(TransformComponent));
            }

            bool hasCameraComponent = entity.HasComponent<CameraComponent>();
            ofs.write((char*)&hasCameraComponent, sizeof(bool));

            if (hasCameraComponent)
            {
                auto& cc = entity.GetComponent<CameraComponent>();
                ofs.write((char*)&cc, sizeof(CameraComponent));
            }

            bool hasMeshComponent = entity.HasComponent<MeshComponent>();
            ofs.write((char*)&hasMeshComponent, sizeof(bool));

            if (hasMeshComponent)
            {
                auto& mc = entity.GetComponent<MeshComponent>();

                UUID uuid = mc.Mesh ? mc.Mesh->GetUUID() : 0;
                ofs.write((char*)&uuid, sizeof(UUID));
            }

            bool hasAnimatedMeshComponent = entity.HasComponent<AnimatedMeshComponent>();
            ofs.write((char*)&hasAnimatedMeshComponent, sizeof(bool));

            if (hasAnimatedMeshComponent)
            {
                auto& amc = entity.GetComponent<AnimatedMeshComponent>();

                UUID meshUUID = amc.Mesh ? amc.Mesh->GetUUID() : 0;
                ofs.write((char*)&meshUUID, sizeof(UUID));

                UUID skeletonUUID = amc.Skeleton ? amc.Skeleton->GetUUID() : 0;
                ofs.write((char*)&skeletonUUID, sizeof(UUID));
            }

            bool hasAnimatorComponent = entity.HasComponent<AnimatorComponent>();
            ofs.write((char*)&hasAnimatorComponent, sizeof(bool));

            if (hasAnimatorComponent)
            {
                auto& ac = entity.GetComponent<AnimatorComponent>();

                UUID animationControllerUUID = ac.AnimationController ? ac.AnimationController->GetUUID() : 0;
                ofs.write((char*)&animationControllerUUID, sizeof(UUID));
                ofs.write((char*)&ac.Play, sizeof(bool));
            }

            bool hasSkyLightComponent = entity.HasComponent<SkyLightComponent>();
            ofs.write((char*)&hasSkyLightComponent, sizeof(bool));

            if (hasSkyLightComponent)
            {
                auto& slc = entity.GetComponent<SkyLightComponent>();

                UUID uuid = slc.EnvironmentMap ? slc.EnvironmentMap->GetUUID() : 0;
                ofs.write((char*)&uuid, sizeof(UUID));
            }

            bool hasDirectionalLightComponent = entity.HasComponent<DirectionalLightComponent>();
            ofs.write((char*)&hasDirectionalLightComponent, sizeof(bool));

            if (hasDirectionalLightComponent)
            {
                auto& dlc = entity.GetComponent<DirectionalLightComponent>();
                ofs.write((char*)&dlc, sizeof(DirectionalLightComponent));
            }

            bool hasPointLightComponent = entity.HasComponent<PointLightComponent>();
            ofs.write((char*)&hasPointLightComponent, sizeof(bool));

            if (hasPointLightComponent)
            {
                auto& plc = entity.GetComponent<PointLightComponent>();
                ofs.write((char*)&plc, sizeof(PointLightComponent));
            }

            bool hasSpotLightComponent = entity.HasComponent<SpotLightComponent>();
            ofs.write((char*)&hasSpotLightComponent, sizeof(bool));

            if (hasSpotLightComponent)
            {
                auto& slc = entity.GetComponent<SpotLightComponent>();
                ofs.write((char*)&slc, sizeof(SpotLightComponent));
            }

            bool hasScriptComponent = entity.HasComponent<ScriptComponent>();
            ofs.write((char*)&hasScriptComponent, sizeof(bool));

            if (hasScriptComponent)
            {
                auto& sc = entity.GetComponent<ScriptComponent>();
                u32 scriptClassSize = sc.ScriptClass.size();
                ofs.write((char*)&scriptClassSize, sizeof(u32));
                ofs.write((char*)sc.ScriptClass.data(), scriptClassSize);

                if (Ref<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(sc.ScriptClass))
                {
                    // Get the fields from the script field map for the entity
                    ScriptVariableMap& scriptInstanceVarMap = ScriptEngine::GetScriptVariableMap(entity);

                    u32 scriptVariableCount = scriptInstanceVarMap.size();
                    ofs.write((char*)&scriptVariableCount, sizeof(u32));

                    for (auto& [name, variable] : scriptInstanceVarMap)
                    {
                        u32 varNameSize = name.size();
                        ofs.write((char*)&varNameSize, sizeof(u32));
                        ofs.write((char*)name.data(), varNameSize);

                        ScriptVariableType type = variable.GetType();
                        ofs.write((char*)&type, sizeof(ScriptVariableType));

                        switch (type)
                        {
                            case ScriptVariableType::Int:
                            {
                                s32 value = variable.GetValue<s32>();
                                ofs.write((char*)&value, sizeof(s32));
                                break;
                            }
                            case ScriptVariableType::Float:
                            {
                                f32 value = variable.GetValue<f32>();
                                ofs.write((char*)&value, sizeof(f32));
                                break;
                            }
                            case ScriptVariableType::Bool:
                            {
                                bool value = variable.GetValue<bool>();
                                ofs.write((char*)&value, sizeof(bool));
                                break;
                            }
                            case ScriptVariableType::Vec2:
                            {
                                glm::vec2 value = variable.GetValue<glm::vec2>();
                                ofs.write((char*)&value, sizeof(glm::vec2));
                                break;
                            }
                            case ScriptVariableType::Vec3:
                            {
                                glm::vec3 value = variable.GetValue<glm::vec3>();
                                ofs.write((char*)&value, sizeof(glm::vec3));
                                break;
                            }
                            case ScriptVariableType::Vec4:
                            {
                                glm::vec4 value = variable.GetValue<glm::vec4>();
                                ofs.write((char*)&value, sizeof(glm::vec4));
                                break;
                            }
                            case ScriptVariableType::Entity:
                            case ScriptVariableType::Material:
                            case ScriptVariableType::Mesh:
                            case ScriptVariableType::Texture2D:
                            case ScriptVariableType::TextureCube:
                            {
                                UUID value = variable.GetValue<UUID>();
                                ofs.write((char*)&value, sizeof(UUID));
                                break;
                            }
                        }
                    }
                }
            }

            bool hasRigidbodyComponent = entity.HasComponent<RigidbodyComponent>();
            ofs.write((char*)&hasRigidbodyComponent, sizeof(bool));

            if (hasRigidbodyComponent)
            {
                auto& rbc = entity.GetComponent<RigidbodyComponent>();
                ofs.write((char*)&rbc, sizeof(RigidbodyComponent));
            }

            bool hasBoxColliderComponent = entity.HasComponent<BoxColliderComponent>();
            ofs.write((char*)&hasBoxColliderComponent, sizeof(bool));

            if (hasBoxColliderComponent)
            {
                auto& bcc = entity.GetComponent<BoxColliderComponent>();
                ofs.write((char*)&bcc, sizeof(BoxColliderComponent));
            }
        });

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    static bool AssetSerializer::Serialize(const std::filesystem::path& filepath, Ref<Animation> asset)
    {
        std::ofstream ofs(filepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        std::filesystem::path absolutePath = std::filesystem::canonical(filepath);

        if (asset->GetAssetFlag(AssetFlags::Serialized) && asset->m_MetaData.AssetFilepath != absolutePath)
        {
            // If the asset was already serialized but the path is different than the one passed as a paraeter, create a copy of the asset with a new ID
            AssetMetaData newMetaData = asset->m_MetaData;
            newMetaData.UUID = UUID();
            newMetaData.AssetFilepath = absolutePath;
            SerializeMetaData(ofs, newMetaData);
        }
        else
        {
            asset->m_MetaData.AssetFilepath = absolutePath;
            asset->SetAssetFlag(AssetFlags::Serialized);
            SerializeMetaData(ofs, asset->m_MetaData);
        }

        ofs.write((char*)&asset->m_Duration, sizeof(f32));
        ofs.write((char*)&asset->m_TicksPerSecond, sizeof(f32));

        u32 keyFrameCount = asset->m_KeyFrames.size();
        ofs.write((char*)&keyFrameCount, sizeof(u32));

        for (auto& keyFrame : asset->m_KeyFrames)
        {
            ofs.write((char*)&keyFrame.TimeStamp, sizeof(f32));

            u32 boneCount = keyFrame.BoneTransforms.size();
            ofs.write((char*)&boneCount, sizeof(u32));

            for (auto& [boneID, boneTransform] : keyFrame.BoneTransforms)
            {
                ofs.write((char*)&boneID, sizeof(u32));
                ofs.write((char*)&boneTransform, sizeof(Animation::BoneTransform));
            }
        }

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    static bool AssetSerializer::Serialize(const std::filesystem::path& filepath, Ref<AnimationController> asset)
    {
        std::ofstream ofs(filepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        std::filesystem::path absolutePath = std::filesystem::canonical(filepath);

        if (asset->GetAssetFlag(AssetFlags::Serialized) && asset->m_MetaData.AssetFilepath != absolutePath)
        {
            // If the asset was already serialized but the path is different than the one passed as a paraeter, create a copy of the asset with a new ID
            AssetMetaData newMetaData = asset->m_MetaData;
            newMetaData.UUID = UUID();
            newMetaData.AssetFilepath = absolutePath;
            SerializeMetaData(ofs, newMetaData);
        }
        else
        {
            asset->m_MetaData.AssetFilepath = absolutePath;
            asset->SetAssetFlag(AssetFlags::Serialized);
            SerializeMetaData(ofs, asset->m_MetaData);
        }

        ofs.write((char*)&asset->m_InitialStateIdx, sizeof(u16));

        u32 animationStatesCount = asset->m_AnimationStates.size();
        ofs.write((char*)&animationStatesCount, sizeof(u32));

        for (const auto& animState : asset->m_AnimationStates)
        {
            UUID uuid = animState->GetUUID();
            ofs.write((char*)&uuid, sizeof(UUID));
        }

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    static bool AssetSerializer::Serialize(const std::filesystem::path& filepath, Ref<Skeleton> asset)
    {
        std::ofstream ofs(filepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        std::filesystem::path absolutePath = std::filesystem::canonical(filepath);

        if (asset->GetAssetFlag(AssetFlags::Serialized) && asset->m_MetaData.AssetFilepath != absolutePath)
        {
            // If the asset was already serialized but the path is different than the one passed as a paraeter, create a copy of the asset with a new ID
            AssetMetaData newMetaData = asset->m_MetaData;
            newMetaData.UUID = UUID();
            newMetaData.AssetFilepath = absolutePath;
            SerializeMetaData(ofs, newMetaData);
        }
        else
        {
            asset->m_MetaData.AssetFilepath = absolutePath;
            asset->SetAssetFlag(AssetFlags::Serialized);
            SerializeMetaData(ofs, asset->m_MetaData);
        }

        u32 boneCount = asset->m_Bones.size();
        ofs.write((char*)&boneCount, sizeof(u32));

        for (auto& bone : asset->m_Bones)
        {
            ofs.write((char*)&bone.ID, sizeof(u32));
            ofs.write((char*)&bone.ParentID, sizeof(u32));

            u32 childrenCount = bone.ChildrenIDs.size();
            ofs.write((char*)&childrenCount, sizeof(u32));
            ofs.write((char*)bone.ChildrenIDs.data(), childrenCount * sizeof(u32));

            ofs.write((char*)&bone.InverseBindTransform, sizeof(glm::mat4));
        }

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    Ref<Texture2D> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return nullptr;

        AssetMetaData metaData;
        metaData.AssetFilepath = std::filesystem::canonical(filepath);
        DeserializeMetaData(ifs, metaData);
        ATOM_ENGINE_ASSERT(metaData.Type == AssetType::Texture2D);

        TextureDescription textureDesc;
        bool isReadable;
        Vector<Vector<byte>> pixelData;

        ifs.read((char*)&textureDesc.Format, sizeof(TextureFormat));
        ifs.read((char*)&textureDesc.Width, sizeof(u32));
        ifs.read((char*)&textureDesc.Height, sizeof(u32));
        ifs.read((char*)&textureDesc.MipLevels, sizeof(u32));
        ifs.read((char*)&textureDesc.Filter, sizeof(TextureFilter));
        ifs.read((char*)&textureDesc.Wrap, sizeof(TextureWrap));
        ifs.read((char*)&textureDesc.UsageFlags, sizeof(TextureBindFlags));
        ifs.read((char*)&isReadable, sizeof(bool));

        pixelData.resize(textureDesc.MipLevels);
        for (u32 mip = 0; mip < textureDesc.MipLevels; mip++)
        {
            u32 pixelDataSize;
            ifs.read((char*)&pixelDataSize, sizeof(u32));
            pixelData[mip].resize(pixelDataSize);
            ifs.read((char*)pixelData[mip].data(), pixelDataSize);
        }

        Ref<Texture2D> asset = CreateRef<Texture2D>(textureDesc, pixelData, isReadable, metaData.SourceFilepath.stem().string().c_str());
        asset->m_MetaData = metaData;

        return asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    Ref<TextureCube> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return nullptr;

        AssetMetaData metaData;
        metaData.AssetFilepath = std::filesystem::canonical(filepath);
        DeserializeMetaData(ifs, metaData);
        ATOM_ENGINE_ASSERT(metaData.Type == AssetType::TextureCube);

        TextureDescription textureDesc;
        bool isReadable;
        Vector<Vector<byte>> pixelData[6];

        ifs.read((char*)&textureDesc.Format, sizeof(TextureFormat));
        ifs.read((char*)&textureDesc.Width, sizeof(u32));
        ifs.read((char*)&textureDesc.Height, sizeof(u32));
        ifs.read((char*)&textureDesc.MipLevels, sizeof(u32));
        ifs.read((char*)&textureDesc.Filter, sizeof(TextureFilter));
        ifs.read((char*)&textureDesc.Wrap, sizeof(TextureWrap));
        ifs.read((char*)&textureDesc.UsageFlags, sizeof(TextureBindFlags));
        ifs.read((char*)&isReadable, sizeof(bool));

        for (u32 face = 0; face < 6; face++)
        {
            pixelData[face].resize(textureDesc.MipLevels);

            for (u32 mip = 0; mip < textureDesc.MipLevels; mip++)
            {
                u32 pixelDataSize;
                ifs.read((char*)&pixelDataSize, sizeof(u32));
                pixelData[face][mip].resize(pixelDataSize, 0);
                ifs.read((char*)pixelData[face][mip].data(), pixelDataSize);
            }
        }

        Ref<TextureCube> asset = CreateRef<TextureCube>(textureDesc, pixelData, isReadable, metaData.SourceFilepath.stem().string().c_str());
        asset->m_MetaData = metaData;

        return asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    Ref<Material> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return nullptr;

        AssetMetaData metaData;
        metaData.AssetFilepath = std::filesystem::canonical(filepath);
        DeserializeMetaData(ifs, metaData);
        ATOM_ENGINE_ASSERT(metaData.Type == AssetType::Material);

        // Deserialize shader name
        u32 shaderNameLength;
        ifs.read((char*)&shaderNameLength, sizeof(u32));

        String shaderName(shaderNameLength, '\0');
        ifs.read(shaderName.data(), shaderNameLength);

        Ref<Material> asset = CreateRef<Material>(Renderer::GetShaderLibrary().Get<GraphicsShader>(shaderName), MaterialFlags::None);
        asset->m_MetaData = metaData;

        // Deserialize flags
        MaterialFlags flags;
        ifs.read((char*)&flags, sizeof(MaterialFlags));
        asset->SetFlags(flags);

        // Deserialize uniform buffers
        u32 uniformBufferCount;
        ifs.read((char*)&uniformBufferCount, sizeof(u32));

        for (u32 i = 0; i < uniformBufferCount; i++)
        {
            u32 bufferRegister;
            ifs.read((char*)&bufferRegister, sizeof(u32));

            u32 bufferSize;
            ifs.read((char*)&bufferSize, sizeof(u32));

            Vector<byte> bufferData(bufferSize, 0);
            ifs.read((char*)bufferData.data(), bufferSize);

            asset->m_UniformBuffersData[bufferRegister] = bufferData;
        }

        // Deserialize textures
        u32 textureCount;
        ifs.read((char*)&textureCount, sizeof(u32));

        for (u32 i = 0; i < textureCount; i++)
        {
            u32 textureRegister;
            ifs.read((char*)&textureRegister, sizeof(u32));

            UUID textureHandle;
            ifs.read((char*)&textureHandle, sizeof(u64));
            
            Ref<Texture> texture = textureHandle != 0 ? AssetManager::GetAsset<Texture>(textureHandle, true) : nullptr;
            asset->m_Textures[textureRegister] = texture;
        }

        return asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    Ref<Mesh> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return nullptr;

        AssetMetaData metaData;
        metaData.AssetFilepath = std::filesystem::canonical(filepath);
        DeserializeMetaData(ifs, metaData);
        ATOM_ENGINE_ASSERT(metaData.Type == AssetType::Mesh);

        u32 vertexCount;
        ifs.read((char*)&vertexCount, sizeof(u32));

        MeshDescription meshDesc;

        meshDesc.Positions.resize(vertexCount);
        ifs.read((char*)meshDesc.Positions.data(), sizeof(glm::vec3) * vertexCount);

        meshDesc.UVs.resize(vertexCount);
        ifs.read((char*)meshDesc.UVs.data(), sizeof(glm::vec2) * vertexCount);

        meshDesc.Normals.resize(vertexCount);
        ifs.read((char*)meshDesc.Normals.data(), sizeof(glm::vec3) * vertexCount);

        meshDesc.Tangents.resize(vertexCount);
        ifs.read((char*)meshDesc.Tangents.data(), sizeof(glm::vec3) * vertexCount);

        meshDesc.Bitangents.resize(vertexCount);
        ifs.read((char*)meshDesc.Bitangents.data(), sizeof(glm::vec3) * vertexCount);

        meshDesc.BoneWeights.resize(vertexCount);
        ifs.read((char*)meshDesc.BoneWeights.data(), sizeof(BoneWeight) * vertexCount);

        u32 indexCount;
        ifs.read((char*)&indexCount, sizeof(u32));

        meshDesc.Indices.resize(indexCount);
        ifs.read((char*)meshDesc.Indices.data(), sizeof(u32) * indexCount);

        u32 submeshCount;
        ifs.read((char*)&submeshCount, sizeof(u32));

        meshDesc.Submeshes.resize(submeshCount);
        ifs.read((char*)meshDesc.Submeshes.data(), sizeof(Submesh) * submeshCount);

        bool isReadable;
        ifs.read((char*)&isReadable, sizeof(bool));

        meshDesc.MaterialTable = CreateRef<MaterialTable>();

        for (u32 submeshIdx = 0; submeshIdx < submeshCount; submeshIdx++)
        {
            UUID materialUUID;
            ifs.read((char*)&materialUUID, sizeof(u64));

            meshDesc.MaterialTable->SetMaterial(submeshIdx, AssetManager::GetAsset<Material>(materialUUID, true));
        }

        Ref<Mesh> asset = CreateRef<Mesh>(meshDesc, isReadable);
        asset->m_MetaData = metaData;

        return asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    Ref<Scene> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return nullptr;

        AssetMetaData metaData;
        metaData.AssetFilepath = std::filesystem::canonical(filepath);
        DeserializeMetaData(ifs, metaData);
        ATOM_ENGINE_ASSERT(metaData.Type == AssetType::Scene);

        Ref<Scene> asset = CreateRef<Scene>();
        asset->m_MetaData = metaData;

        u32 nameSize;
        ifs.read((char*)&nameSize, sizeof(u32));

        asset->m_Name.resize(nameSize);
        ifs.read((char*)asset->m_Name.data(), nameSize);

        u32 entityCount;
        ifs.read((char*)&entityCount, sizeof(u32));

        for (u32 i = 0; i < entityCount; i++)
        {
            bool isValid;
            ifs.read((char*)&isValid, sizeof(bool));

            if (!isValid)
                continue;

            UUID uuid;
            ifs.read((char*)&uuid, sizeof(UUID));

            Entity entity = asset->CreateEntityFromUUID(uuid);

            bool hasTagComponent;
            ifs.read((char*)&hasTagComponent, sizeof(bool));

            if (hasTagComponent)
            {
                auto& tc = entity.AddOrReplaceComponent<TagComponent>();

                u32 tagSize;
                ifs.read((char*)&tagSize, sizeof(u32));

                tc.Tag.resize(tagSize);
                ifs.read((char*)tc.Tag.data(), tagSize);
            }

            bool hasSceneHierarchyComponent;
            ifs.read((char*)&hasSceneHierarchyComponent, sizeof(bool));

            if (hasSceneHierarchyComponent)
            {
                auto& shc = entity.AddOrReplaceComponent<SceneHierarchyComponent>();
                ifs.read((char*)&shc, sizeof(SceneHierarchyComponent));
            }

            bool hasTransformComponent;
            ifs.read((char*)&hasTransformComponent, sizeof(bool));

            if (hasTransformComponent)
            {
                auto& tc = entity.AddOrReplaceComponent<TransformComponent>();
                ifs.read((char*)&tc, sizeof(TransformComponent));
            }

            bool hasCameraComponent;
            ifs.read((char*)&hasCameraComponent, sizeof(bool));

            if (hasCameraComponent)
            {
                auto& cc = entity.AddOrReplaceComponent<CameraComponent>();
                ifs.read((char*)&cc, sizeof(CameraComponent));
            }

            bool hasMeshComponent;
            ifs.read((char*)&hasMeshComponent, sizeof(bool));

            if (hasMeshComponent)
            {
                auto& mc = entity.AddOrReplaceComponent<MeshComponent>();

                UUID uuid;
                ifs.read((char*)&uuid, sizeof(UUID));

                mc.Mesh = AssetManager::GetAsset<Mesh>(uuid, true);
            }

            bool hasAnimatedMeshComponent;
            ifs.read((char*)&hasAnimatedMeshComponent, sizeof(bool));

            if (hasAnimatedMeshComponent)
            {
                auto& amc = entity.AddOrReplaceComponent<AnimatedMeshComponent>();

                UUID meshUUID;
                ifs.read((char*)&meshUUID, sizeof(UUID));

                UUID skeletonUUID;
                ifs.read((char*)&skeletonUUID, sizeof(UUID));

                amc.Mesh = AssetManager::GetAsset<Mesh>(meshUUID, true);
                amc.Skeleton = AssetManager::GetAsset<Skeleton>(skeletonUUID, true);
            }

            bool hasAnimatorComponent;
            ifs.read((char*)&hasAnimatorComponent, sizeof(bool));

            if (hasAnimatorComponent)
            {
                auto& ac = entity.AddOrReplaceComponent<AnimatorComponent>();

                UUID animationControllerUUID;
                ifs.read((char*)&animationControllerUUID, sizeof(UUID));

                ac.AnimationController = AssetManager::GetAsset<AnimationController>(animationControllerUUID, true);

                ifs.read((char*)&ac.Play, sizeof(bool));
            }

            bool hasSkyLightComponent;
            ifs.read((char*)&hasSkyLightComponent, sizeof(bool));

            if (hasSkyLightComponent)
            {
                auto& slc = entity.AddOrReplaceComponent<SkyLightComponent>();

                UUID uuid;
                ifs.read((char*)&uuid, sizeof(UUID));

                slc.EnvironmentMap = AssetManager::GetAsset<TextureCube>(uuid, true);
            }

            bool hasDirectionalLightComponent;
            ifs.read((char*)&hasDirectionalLightComponent, sizeof(bool));

            if (hasDirectionalLightComponent)
            {
                auto& dlc = entity.AddOrReplaceComponent<DirectionalLightComponent>();
                ifs.read((char*)&dlc, sizeof(DirectionalLightComponent));
            }

            bool hasPointLightComponent;
            ifs.read((char*)&hasPointLightComponent, sizeof(bool));

            if (hasPointLightComponent)
            {
                auto& plc = entity.AddOrReplaceComponent<PointLightComponent>();
                ifs.read((char*)&plc, sizeof(PointLightComponent));
            }

            bool hasSpotLightComponent;
            ifs.read((char*)&hasSpotLightComponent, sizeof(bool));

            if (hasSpotLightComponent)
            {
                auto& slc = entity.AddOrReplaceComponent<SpotLightComponent>();
                ifs.read((char*)&slc, sizeof(SpotLightComponent));
            }

            bool hasScriptComponent;
            ifs.read((char*)&hasScriptComponent, sizeof(bool));

            if (hasScriptComponent)
            {
                auto& sc = entity.AddOrReplaceComponent<ScriptComponent>();

                u32 scriptClassSize;
                ifs.read((char*)&scriptClassSize, sizeof(u32));

                sc.ScriptClass.resize(scriptClassSize);
                ifs.read((char*)sc.ScriptClass.data(), scriptClassSize);

                if (Ref<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(sc.ScriptClass))
                {
                    ScriptVariableMap& scriptInstanceVarMap = ScriptEngine::GetScriptVariableMap(entity);

                    u32 scriptVariableCount;
                    ifs.read((char*)&scriptVariableCount, sizeof(u32));

                    for (u32 i = 0; i < scriptVariableCount; i++)
                    {
                        u32 varNameSize;
                        ifs.read((char*)&varNameSize, sizeof(u32));

                        String varName(varNameSize, '\0');
                        ifs.read((char*)varName.data(), varNameSize);

                        ScriptVariableType type;
                        ifs.read((char*)&type, sizeof(ScriptVariableType));

                        ScriptVariable variable(varName, type);

                        switch (type)
                        {
                            case ScriptVariableType::Int:
                            {
                                s32 value;
                                ifs.read((char*)&value, sizeof(s32));
                                variable.SetValue(value);
                                break;
                            }
                            case ScriptVariableType::Float:
                            {
                                f32 value;
                                ifs.read((char*)&value, sizeof(f32));
                                variable.SetValue(value);
                                break;
                            }
                            case ScriptVariableType::Bool:
                            {
                                bool value;
                                ifs.read((char*)&value, sizeof(bool));
                                variable.SetValue(value);
                                break;
                            }
                            case ScriptVariableType::Vec2:
                            {
                                glm::vec2 value;
                                ifs.read((char*)&value, sizeof(glm::vec2));
                                variable.SetValue(value);
                                break;
                            }
                            case ScriptVariableType::Vec3:
                            {
                                glm::vec3 value;
                                ifs.read((char*)&value, sizeof(glm::vec3));
                                variable.SetValue(value);
                                break;
                            }
                            case ScriptVariableType::Vec4:
                            {
                                glm::vec4 value;
                                ifs.read((char*)&value, sizeof(glm::vec4));
                                variable.SetValue(value);
                                break;
                            }
                            case ScriptVariableType::Entity:
                            case ScriptVariableType::Material:
                            case ScriptVariableType::Mesh:
                            case ScriptVariableType::Texture2D:
                            case ScriptVariableType::TextureCube:
                            {
                                UUID value;
                                ifs.read((char*)&value, sizeof(UUID));
                                variable.SetValue(value);
                                break;
                            }
                        }

                        scriptInstanceVarMap[varName] = variable;
                    }
                }
            }

            bool hasRigidbodyComponent;
            ifs.read((char*)&hasRigidbodyComponent, sizeof(bool));

            if (hasRigidbodyComponent)
            {
                auto& rbc = entity.AddOrReplaceComponent<RigidbodyComponent>();
                ifs.read((char*)&rbc, sizeof(RigidbodyComponent));
            }

            bool hasBoxColliderComponent;
            ifs.read((char*)&hasBoxColliderComponent, sizeof(bool));

            if (hasBoxColliderComponent)
            {
                auto& bcc = entity.AddOrReplaceComponent<BoxColliderComponent>();
                ifs.read((char*)&bcc, sizeof(BoxColliderComponent));
            }
        }

        return asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    Ref<Animation> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return nullptr;

        AssetMetaData metaData;
        metaData.AssetFilepath = std::filesystem::canonical(filepath);
        DeserializeMetaData(ifs, metaData);
        ATOM_ENGINE_ASSERT(metaData.Type == AssetType::Animation);

        f32 duration;
        ifs.read((char*)&duration, sizeof(f32));

        f32 ticksPerSecond;
        ifs.read((char*)&ticksPerSecond, sizeof(f32));

        u32 keyFrameCount;
        ifs.read((char*)&keyFrameCount, sizeof(u32));

        Set<Animation::KeyFrame> keyFrames;

        for (u32 keyFrameIdx = 0; keyFrameIdx < keyFrameCount; keyFrameIdx++)
        {
            Animation::KeyFrame keyFrame;
            ifs.read((char*)&keyFrame.TimeStamp, sizeof(f32));

            u32 boneCount;
            ifs.read((char*)&boneCount, sizeof(u32));
            keyFrame.BoneTransforms.reserve(boneCount);

            for (u32 boneIdx = 0; boneIdx < boneCount; boneIdx++)
            {
                u32 boneID;
                ifs.read((char*)&boneID, sizeof(u32));

                Animation::BoneTransform boneTransform;
                ifs.read((char*)&boneTransform, sizeof(Animation::BoneTransform));

                keyFrame.BoneTransforms[boneID] = boneTransform;
            }

            keyFrames.insert(keyFrame);
        }

        Ref<Animation> asset = CreateRef<Animation>(duration, ticksPerSecond, keyFrames);
        asset->m_MetaData = metaData;

        return asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    Ref<AnimationController> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return nullptr;

        AssetMetaData metaData;
        metaData.AssetFilepath = std::filesystem::canonical(filepath);
        DeserializeMetaData(ifs, metaData);
        ATOM_ENGINE_ASSERT(metaData.Type == AssetType::AnimationController);

        u16 initialStateIdx;
        ifs.read((char*)&initialStateIdx, sizeof(u16));

        u32 animationStatesCount;
        ifs.read((char*)&animationStatesCount, sizeof(u32));

        Vector<Ref<Animation>> animationStates;
        animationStates.reserve(animationStatesCount);

        for (u32 i = 0 ; i < animationStatesCount; i++)
        {
            UUID uuid;
            ifs.read((char*)&uuid, sizeof(UUID));
            animationStates.push_back(AssetManager::GetAsset<Animation>(uuid, true));
        }

        Ref<AnimationController> asset = CreateRef<AnimationController>(animationStates, initialStateIdx);
        asset->m_MetaData = metaData;

        return asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    Ref<Skeleton> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return nullptr;

        AssetMetaData metaData;
        metaData.AssetFilepath = std::filesystem::canonical(filepath);
        DeserializeMetaData(ifs, metaData);
        ATOM_ENGINE_ASSERT(metaData.Type == AssetType::Skeleton);

        u32 boneCount;
        ifs.read((char*)&boneCount, sizeof(u32));

        Vector<Skeleton::Bone> bones;
        bones.reserve(boneCount);

        for (u32 i = 0; i < boneCount; i++)
        {
            Skeleton::Bone& bone = bones.emplace_back();
            ifs.read((char*)&bone.ID, sizeof(u32));
            ifs.read((char*)&bone.ParentID, sizeof(u32));

            u32 childrenCount = bone.ChildrenIDs.size();
            ifs.read((char*)&childrenCount, sizeof(u32));

            bone.ChildrenIDs.resize(childrenCount);
            ifs.read((char*)bone.ChildrenIDs.data(), childrenCount * sizeof(u32));

            ifs.read((char*)&bone.InverseBindTransform, sizeof(glm::mat4));
        }

        Ref<Skeleton> asset = CreateRef<Skeleton>(bones);
        asset->m_MetaData = metaData;

        return asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool AssetSerializer::DeserializeMetaData(const std::filesystem::path& filepath, AssetMetaData& assetMetaData)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return false;

        DeserializeMetaData(ifs, assetMetaData);
        assetMetaData.AssetFilepath = std::filesystem::canonical(filepath);

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetSerializer::SerializeMetaData(std::ofstream& stream, const AssetMetaData& metaData)
    {

        String sourcePathStr = metaData.SourceFilepath.string();
        u32 sourcePathSize = sourcePathStr.size();
        stream.write((char*)&metaData.UUID, sizeof(u64));
        stream.write((char*)&metaData.Type, sizeof(AssetType));
        stream.write((char*)&metaData.Flags, sizeof(AssetFlags));
        stream.write((char*)&sourcePathSize, sizeof(u32));
        stream.write(sourcePathStr.data(), sourcePathSize);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetSerializer::DeserializeMetaData(std::ifstream& stream, AssetMetaData& metaData)
    {
        u32 sourcePathSize;
        String sourcePathStr;
        stream.read((char*)&metaData.UUID, sizeof(u64));
        stream.read((char*)&metaData.Type, sizeof(AssetType));
        stream.read((char*)&metaData.Flags, sizeof(AssetFlags));
        stream.read((char*)&sourcePathSize, sizeof(u32));
        sourcePathStr.resize(sourcePathSize);
        stream.read(sourcePathStr.data(), sourcePathSize);
        metaData.SourceFilepath = sourcePathStr;
    }
}
