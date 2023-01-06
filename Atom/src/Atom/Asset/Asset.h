#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/UUID.h"

namespace Atom
{
    enum class AssetType
    {
        None = 0,
        Texture2D,
        TextureCube,
        Mesh,
        Material,
        Scene
    };

    enum class AssetFlags
    {
        None = 0,
        Serialized
    };

    IMPL_ENUM_OPERATORS(AssetFlags)

    struct AssetMetaData
    {
        UUID                  UUID;
        AssetType             Type;
        AssetFlags            Flags;
        std::filesystem::path SourceFilepath;
        std::filesystem::path AssetFilepath;
    };

    class Asset
    {
        friend class AssetSerializer;
        friend class ContentTools;
    public:
        inline static const char* AssetTypeExtension = ".atmasset";
    public:
        virtual ~Asset() = default;

        inline void SetAssetFlag(AssetFlags flag, bool state = true) { if (state) m_MetaData.Flags |= flag; else m_MetaData.Flags &= ~flag; }
        inline UUID GetUUID() const { return m_MetaData.UUID; }
        inline AssetType GetAssetType() const { return m_MetaData.Type; }
        inline bool GetAssetFlag(AssetFlags flag) const { return (m_MetaData.Flags & flag) != AssetFlags::None; }
        inline AssetFlags GetAssetFlags() const { return m_MetaData.Flags; }
        inline const std::filesystem::path& GetAssetFilepath() const { return m_MetaData.AssetFilepath; }
        inline const std::filesystem::path& GetSourceFilepath() const { return m_MetaData.SourceFilepath; }
        inline const AssetMetaData& GetMetaData() const { return m_MetaData; }
    protected:
        Asset(AssetType type, AssetFlags flags = AssetFlags::None)
        {
            m_MetaData.Type = type;
            m_MetaData.Flags = flags;
        }
    protected:
        AssetMetaData m_MetaData;
    };
}