#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <variant>
#include <sstream>

namespace SIGCompiler
{
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
        std::variant<SIGConstantType, std::string> ReturnType;
        uint32_t ArraySize;
    };

    struct SIGSampler
    {
        std::string Name;
        uint32_t ShaderRegister;
        uint32_t ArraySize;
    };

    struct SIGBindPoint
    {
        std::string Name;
        std::string LayoutName;
        std::string FullName;
        uint32_t ShaderSpace = UINT32_MAX;

        uint32_t ConstantBufferRootIdx = UINT32_MAX;
        uint32_t ResourceTableRootIdx = UINT32_MAX;
        uint32_t SamplerTableRootIdx = UINT32_MAX;

        uint32_t ConstantBufferSize = 0;
        uint32_t NumSRVs = 0;
        uint32_t NumUAVs = 0;
        uint32_t NumSamplers = 0;
    };

    enum class SamplerFilter
    {
        None = 0,
        Linear, Nearest, Anisotropic
    };

    enum class SamplerWrap
    {
        None = 0,
        Clamp, Repeat
    };

    struct SIGStaticSampler
    {
        std::string Name;
        std::string LayoutName;
        std::string FullName;
        uint32_t ShaderSpace = UINT32_MAX;

        uint32_t ShaderRegister;

        SamplerFilter Filter;
        SamplerWrap Wrap;
    };

    class SIGDeclaration
    {
    public:
        SIGDeclaration(const std::string& name, const std::string& bindPointFullName);

        bool AddConstant(const std::string& name, SIGConstantType type);
        bool AddResource(const std::string& name, SIGResourceType type, const std::variant<SIGConstantType, std::string>& returnType, uint32_t arraySize = 1);
        bool AddSampler(const std::string& name, uint32_t arraySize = 1);

        inline const std::string& GetName() const { return m_Name; }
        inline const std::string& GetBindPointFullName() const { return m_BindPointFullName; }
        inline const std::vector<SIGConstant>& GetConstants() const { return m_Constants; }
        inline const std::vector<SIGResource>& GetResources() const { return m_Resources; }
        inline const std::vector<SIGSampler>& GetSamplers() const { return m_Samplers; }
        inline uint32_t GetConstantBufferSize() const { return m_ConstantBufferSize; }
        inline uint32_t GetNumSRVs() const { return m_NumSRVs; }
        inline uint32_t GetNumUAVs() const { return m_NumUAVs; }
        inline uint32_t GetNumSamplers() const { return m_NumSamplers; }
    private:
        std::string m_Name;
        std::string m_BindPointFullName;
        std::vector<SIGConstant> m_Constants;
        std::vector<SIGResource> m_Resources;
        std::vector<SIGSampler> m_Samplers;
        std::unordered_set<std::string> m_ExistingNames;

        uint32_t m_ConstantBufferSize = 0;
        uint32_t m_NumSRVs = 0;
        uint32_t m_NumUAVs = 0;
        uint32_t m_NumSamplers = 0;
    };

    class SIGLayoutDeclaration
    {
    public:
        SIGLayoutDeclaration(const std::string& name);

        bool AddBindPoint(const std::string& bindPointFullName);
        bool AddStaticSampler(const std::string& staticSamplerFullName);

        inline const std::string& GetName() const { return m_Name; }
        inline const std::vector<std::string>& GetBindPointFullNames() const { return m_BindPointFullNames; }
        inline const std::vector<std::string>& GetStaticSamplerFullNames() const { return m_StaticSamplerFullNames; }
    private:
        std::string m_Name;
        std::vector<std::string> m_BindPointFullNames;
        std::vector<std::string> m_StaticSamplerFullNames;
    };
}