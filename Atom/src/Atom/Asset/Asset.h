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

    class Asset
    {
        friend class AssetSerializer;
        friend class ContentTools;
    public:
        inline static const char* AssetTypeExtension = ".atmasset";
    public:
        virtual ~Asset() = default;

        inline UUID GetUUID() const { return m_UUID; }
        inline AssetType GetAssetType() const { return m_AssetType; }
        inline const std::filesystem::path& GetAssetFilepath() const { return m_AssetFilepath; }
        inline const std::filesystem::path& GetSourceFilepath() const { return m_SourceFilepath; }
    protected:
        Asset(AssetType type)
            : m_AssetType(type) {}
    protected:
        UUID                  m_UUID;
        AssetType             m_AssetType;
        std::filesystem::path m_AssetFilepath;
        std::filesystem::path m_SourceFilepath;
    };
}