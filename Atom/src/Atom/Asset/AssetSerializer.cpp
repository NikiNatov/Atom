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
        std::ofstream ofs(asset->m_MetaData.AssetFilepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        asset->SetAssetFlag(AssetFlags::Serialized);
        SerializeMetaData(ofs, asset->m_MetaData);

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
        std::ofstream ofs(asset->m_MetaData.AssetFilepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        asset->SetAssetFlag(AssetFlags::Serialized);
        SerializeMetaData(ofs, asset->m_MetaData);

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
    static bool AssetSerializer::Serialize(Ref<Material> asset)
    {
        std::ofstream ofs(asset->m_MetaData.AssetFilepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        asset->SetAssetFlag(AssetFlags::Serialized);
        SerializeMetaData(ofs, asset->m_MetaData);

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
    static bool AssetSerializer::Serialize(Ref<Mesh> asset)
    {
        std::ofstream ofs(asset->m_MetaData.AssetFilepath, std::ios::out | std::ios::binary);

        if (!ofs)
            return false;

        asset->SetAssetFlag(AssetFlags::Serialized);
        SerializeMetaData(ofs, asset->m_MetaData);

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

            if (Ref<Material> material = asset->m_MaterialTable->GetMaterial(submeshIdx))
                materialUUID = material->m_MetaData.UUID;

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

        AssetMetaData metaData;
        metaData.AssetFilepath = filepath;
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
        metaData.AssetFilepath = filepath;
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
        metaData.AssetFilepath = filepath;
        DeserializeMetaData(ifs, metaData);
        ATOM_ENGINE_ASSERT(metaData.Type == AssetType::Material);

        Ref<Material> asset = CreateRef<Material>(Renderer::GetShaderLibrary().Get<GraphicsShader>("MeshPBRShader"), MaterialFlags::None);
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
        metaData.AssetFilepath = filepath;
        DeserializeMetaData(ifs, metaData);
        ATOM_ENGINE_ASSERT(metaData.Type == AssetType::Mesh);

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

        Ref<MaterialTable> materialTable = CreateRef<MaterialTable>();

        for (u32 submeshIdx = 0; submeshIdx < submeshCount; submeshIdx++)
        {
            UUID materialUUID;
            ifs.read((char*)&materialUUID, sizeof(u64));

            materialTable->SetMaterial(submeshIdx, AssetManager::GetAsset<Material>(materialUUID, true));
        }

        Ref<Mesh> asset = CreateRef<Mesh>(vertices, indices, submeshes, materialTable, isReadable);
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
        assetMetaData.AssetFilepath = filepath;

        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void AssetSerializer::SerializeMetaData(std::ofstream& stream, const AssetMetaData& metaData)
    {

        String sourcePathStr = metaData.SourceFilepath.string();
        u32 sourcePathSize = sourcePathStr.size() + 1;
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
