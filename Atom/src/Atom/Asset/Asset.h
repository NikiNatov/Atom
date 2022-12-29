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
    };

    enum class AssetFlags
    {
        None = 0,
        Serialized
    };

    IMPL_ENUM_OPERATORS(AssetFlags)

    class Asset
    {
        friend class AssetSerializer;
        friend class ContentTools;
    public:
        inline static const char* AssetTypeExtension = ".atmasset";
    public:
        virtual ~Asset() = default;

        inline void SetAssetFlag(AssetFlags flag, bool state = true) { if (state) m_AssetFlags |= flag; else m_AssetFlags &= ~flag; }
        inline UUID GetUUID() const { return m_UUID; }
        inline AssetType GetAssetType() const { return m_AssetType; }
        inline bool GetAssetFlag(AssetFlags flag) const { return (m_AssetFlags & flag) != AssetFlags::None; }
        inline AssetFlags GetAssetFlags() const { return m_AssetFlags; }
        inline const std::filesystem::path& GetAssetFilepath() const { return m_AssetFilepath; }
        inline const std::filesystem::path& GetSourceFilepath() const { return m_SourceFilepath; }
    protected:
        Asset(AssetType type, AssetFlags flags = AssetFlags::None)
            : m_AssetType(type), m_AssetFlags(flags) {}
    protected:
        UUID                  m_UUID;
        AssetType             m_AssetType;
        AssetFlags            m_AssetFlags;
        std::filesystem::path m_AssetFilepath;
        std::filesystem::path m_SourceFilepath;
    };
}