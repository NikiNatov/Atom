#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <variant>

namespace SIGCompiler
{
    enum class SIGBindPoint
    {
        None = 0,
        Instance,
        Material,
        Pass,
        Frame,
    };

    enum class SIGConstantType
    {
        None = 0,
        Int, Int2, Int3, Int4,
        Uint, Uint2, Uint3, Uint4,
        Float, Float2, Float3, Float4,
        Bool,
        Matrix
    };

    enum class SIGResourceType
    {
        None = 0,
        Texture2D, Texture2DArray, Texture3D, TextureCube, StructuredBuffer,
        RWTexture2D, RWTexture2DArray, RWTexture3D, RWStructuredBuffer,
    };

    struct SIGConstant
    {
        std::string Name;
        SIGConstantType Type;
        uint32_t SizeInBytes;
    };

    class SIGStructType
    {
    public:
        SIGStructType(const std::string& name);

        bool AddMember(const std::string& name, SIGConstantType type);

        inline const std::string& GetName() const { return m_Name; }
        inline uint32_t GetSizeInBytes() const { return m_SizeInBytes; }
        inline const std::vector<SIGConstant>& GetMembers() const { return m_Members; }
    private:
        std::string m_Name;
        uint32_t m_SizeInBytes;
        std::vector<SIGConstant> m_Members;
        std::unordered_set<std::string> m_ExistingNames;
    };

    struct SIGResource
    {
        std::string Name;
        SIGResourceType Type;
        uint32_t ShaderRegister;
        std::variant<SIGConstantType, SIGStructType> ReturnType;
        uint32_t ArraySize;
    };

    struct SIGSampler
    {
        std::string Name;
        uint32_t ShaderRegister;
        uint32_t ArraySize;
    };

    class SIGDeclaration
    {
    public:
        SIGDeclaration(const std::string& name, SIGBindPoint bindPoint);

        bool AddConstant(const std::string& name, SIGConstantType type);
        bool AddResource(const std::string& name, SIGResourceType type, const std::variant<SIGConstantType, SIGStructType>& returnType, uint32_t arraySize = 1);
        bool AddSampler(const std::string& name, uint32_t arraySize = 1);

        inline const std::string& GetName() const { return m_Name; }
        inline SIGBindPoint GetBindPoint() const { return m_BindPoint; }
        inline const std::vector<SIGConstant>& GetConstants() const { return m_Constants; }
        inline const std::vector<SIGResource>& GetResources() const { return m_Resources; }
        inline const std::vector<SIGSampler>& GetSamplers() const { return m_Samplers; }
    private:
        std::string m_Name;
        SIGBindPoint m_BindPoint;
        std::vector<SIGConstant> m_Constants;
        std::vector<SIGResource> m_Resources;
        std::vector<SIGSampler> m_Samplers;
        std::unordered_set<std::string> m_ExistingNames;

        uint32_t m_NumSRVs = 0;
        uint32_t m_NumUAVs = 0;
        uint32_t m_NumSamplers = 0;
    };
}