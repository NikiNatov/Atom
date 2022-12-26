#include "atompch.h"
#include "AssetSerializer.h"

#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/Material.h"
#include "Atom/Renderer/Mesh.h"
#include "Atom/Renderer/Buffer.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    static bool AssetSerializer::Serialize(Ref<Texture2D> asset)
    {
        std::ofstream ofs(asset->m_AssetFilepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        SerializeMetaData(ofs, asset);

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
    static bool AssetSerializer::Serialize(Ref<TextureCube> asset)
    {
        std::ofstream ofs(asset->m_AssetFilepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        SerializeMetaData(ofs, asset);

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
    static bool AssetSerializer::Serialize(Ref<MaterialAsset> asset)
    {
        std::ofstream ofs(asset->m_AssetFilepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        SerializeMetaData(ofs, asset);

        // Serialize flags
        MaterialFlags flags = asset->GetFlags();
        ofs.write((char*)&flags, sizeof(MaterialFlags));

        // Serialize uniform buffers
        u32 uniformBufferCount = asset->m_Material->GetUniformBuffersData().size();
        ofs.write((char*)&uniformBufferCount, sizeof(u32));

        for (auto& [bufferRegister, bufferData] : asset->m_Material->GetUniformBuffersData())
        {
            u32 bufferSize = bufferData.size();
            ofs.write((char*)&bufferRegister, sizeof(u32));
            ofs.write((char*)&bufferSize, sizeof(u32));
            ofs.write((char*)bufferData.data(), bufferSize);
        }

        // Serialize textures
        u32 textureCount = asset->m_Material->GetTextures().size();
        ofs.write((char*)&textureCount, sizeof(u32));

        for (auto& [textureRegister, texture] : asset->m_Material->GetTextures())
        {
            UUID textureHandle = 0;

            if (texture)
            {
                switch (texture->GetType())
                {
                    case TextureType::Texture2D:
                    {
                        Ref<Texture2D> texture = std::dynamic_pointer_cast<Texture2D>(asset->GetTexture(textureRegister));
                        ATOM_ENGINE_ASSERT(texture);
                        textureHandle = texture->GetUUID();
                        break;
                    }
                    case TextureType::TextureCube:
                    {
                        Ref<TextureCube> texture = std::dynamic_pointer_cast<TextureCube>(asset->GetTexture(textureRegister));
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
    static bool AssetSerializer::Serialize(Ref<Mesh> asset)
    {
        std::ofstream ofs(asset->m_AssetFilepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        SerializeMetaData(ofs, asset);

        u32 vertexCount = asset->m_Vertices.size();
        ofs.write((char*)&vertexCount, sizeof(u32));
        ofs.write((char*)asset->m_Vertices.data(), sizeof(Vertex) * vertexCount);

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

            if (Ref<MaterialAsset> material = asset->m_MaterialTable.GetMaterial(submeshIdx))
                materialUUID = material->m_UUID;

            ofs.write((char*)&materialUUID, sizeof(u64));
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

        UUID uuid;
        AssetType assetType;
        std::filesystem::path sourcePath;
        TextureDescription textureDesc;
        bool isReadable;
        Vector<Vector<byte>> pixelData;

        DeserializeMetaData(ifs, uuid, assetType, sourcePath);
        ATOM_ENGINE_ASSERT(assetType == AssetType::Texture2D);

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

        Ref<Texture2D> asset = CreateRef<Texture2D>(textureDesc, pixelData, isReadable, sourcePath.stem().string().c_str());
        asset->m_UUID = uuid;
        asset->m_AssetType = assetType;
        asset->m_SourceFilepath = sourcePath;
        asset->m_AssetFilepath = filepath;

        return asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    Ref<TextureCube> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return nullptr;

        UUID uuid;
        AssetType assetType;
        std::filesystem::path sourcePath;
        TextureDescription textureDesc;
        bool isReadable;
        Vector<Vector<byte>> pixelData[6];

        DeserializeMetaData(ifs, uuid, assetType, sourcePath);
        ATOM_ENGINE_ASSERT(assetType == AssetType::TextureCube);

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

        Ref<TextureCube> asset = CreateRef<TextureCube>(textureDesc, pixelData, isReadable, sourcePath.stem().string().c_str());
        asset->m_UUID = uuid;
        asset->m_AssetType = assetType;
        asset->m_SourceFilepath = sourcePath;
        asset->m_AssetFilepath = filepath;

        return asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    template<>
    Ref<MaterialAsset> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return nullptr;

        Ref<MaterialAsset> asset = CreateRef<MaterialAsset>(Renderer::GetShaderLibrary().Get<GraphicsShader>("MeshPBRShader"));
        asset->m_AssetFilepath = filepath;
        DeserializeMetaData(ifs, asset->m_UUID, asset->m_AssetType, asset->m_SourceFilepath);
        ATOM_ENGINE_ASSERT(asset->m_AssetType == AssetType::Material);

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

            asset->SetUniformBufferData(bufferRegister, bufferData);
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
            asset->SetTexture(textureRegister, texture);
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

        UUID uuid;
        AssetType assetType;
        std::filesystem::path sourcePath;

        DeserializeMetaData(ifs, uuid, assetType, sourcePath);
        ATOM_ENGINE_ASSERT(assetType == AssetType::Mesh);

        u32 vertexCount;
        ifs.read((char*)&vertexCount, sizeof(u32));

        Vector<Vertex> vertices(vertexCount);
        ifs.read((char*)vertices.data(), sizeof(Vertex) * vertexCount);

        u32 indexCount;
        ifs.read((char*)&indexCount, sizeof(u32));

        Vector<u32> indices(indexCount);
        ifs.read((char*)indices.data(), sizeof(u32) * indexCount);

        u32 submeshCount;
        ifs.read((char*)&submeshCount, sizeof(u32));

        Vector<Submesh> submeshes(submeshCount);
        ifs.read((char*)submeshes.data(), sizeof(Submesh) * submeshCount);

        bool isReadable;
        ifs.read((char*)&isReadable, sizeof(bool));

        MaterialTable materialTable;

        for (u32 submeshIdx = 0; submeshIdx < submeshCount; submeshIdx++)
        {
            UUID materialUUID;
            ifs.read((char*)&materialUUID, sizeof(u64));

            materialTable.SetMaterial(submeshIdx, materialUUID);
        }

        Ref<Mesh> asset = CreateRef<Mesh>(vertices, indices, submeshes, materialTable, isReadable);
        asset->m_UUID = uuid;
        asset->m_AssetType = assetType;
        asset->m_SourceFilepath = sourcePath;
        asset->m_AssetFilepath = filepath;

        return asset;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool AssetSerializer::DeserializeMetaData(const std::filesystem::path& filepath, AssetMetaData& assetMetaData)
    {
        std::ifstream ifs(filepath, std::ios::in | std::ios::binary);

        if (!ifs)
            return false;

        DeserializeMetaData(ifs, assetMetaData.UUID, assetMetaData.Type, assetMetaData.SourceFilepath);
        assetMetaData.AssetFilepath = filepath;

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetSerializer::SerializeMetaData(std::ofstream& stream, Ref<Asset> asset)
    {
        String sourcePathStr = asset->m_SourceFilepath.string();
        u32 sourcePathSize = sourcePathStr.size() + 1;
        stream.write((char*)&asset->m_UUID, sizeof(u64));
        stream.write((char*)&asset->m_AssetType, sizeof(AssetType));
        stream.write((char*)&sourcePathSize, sizeof(u32));
        stream.write(sourcePathStr.data(), sourcePathSize);
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetSerializer::DeserializeMetaData(std::ifstream& stream, UUID& uuid, AssetType& assetType, std::filesystem::path& sourcePath)
    {
        u32 sourcePathSize;
        String sourcePathStr;
        stream.read((char*)&uuid, sizeof(u64));
        stream.read((char*)&assetType, sizeof(AssetType));
        stream.read((char*)&sourcePathSize, sizeof(u32));
        sourcePathStr.resize(sourcePathSize);
        stream.read(sourcePathStr.data(), sourcePathSize);
        sourcePath = sourcePathStr;
    }
}
