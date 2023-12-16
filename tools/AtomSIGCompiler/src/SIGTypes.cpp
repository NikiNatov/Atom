#include "SIGTypes.h"
#include "SIGUtils.h"

#include <sstream>

namespace SIGCompiler
{
    // -----------------------------------------------------------------------------------------------------------------------------
    SIGStructType::SIGStructType(const std::string& name)
        : m_Name(name), m_SizeInBytes(0)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool SIGStructType::AddMember(const std::string& name, SIGConstantType type)
    {
        if (m_ExistingNames.find(name) != m_ExistingNames.end())
            return false;

        SIGConstant& sigConstant = m_Members.emplace_back();
        sigConstant.Name = name;
        sigConstant.Type = type;
        sigConstant.SizeInBytes = Utils::GetSIGConstantSize(type);

        m_ExistingNames.insert(name);
        m_SizeInBytes += sigConstant.SizeInBytes;
        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SIGDeclaration::SIGDeclaration(const std::string& name, const std::string& bindPointFullName)
        : m_Name(name), m_BindPointFullName(bindPointFullName)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool SIGDeclaration::AddConstant(const std::string& name, SIGConstantType type)
    {
        if (m_ExistingNames.find(name) != m_ExistingNames.end())
            return false;

        SIGConstant& sigConstant = m_Constants.emplace_back();
        sigConstant.Name = name;
        sigConstant.Type = type;
        sigConstant.SizeInBytes = Utils::GetSIGConstantSize(type);

        m_ConstantBufferSize += sigConstant.SizeInBytes;
        m_ExistingNames.insert(name);
        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool SIGDeclaration::AddResource(const std::string& name, SIGResourceType type, const std::variant<SIGConstantType, std::string>& returnType, uint32_t arraySize)
    {
        if (m_ExistingNames.find(name) != m_ExistingNames.end())
            return false;

        SIGResource& sigResource = m_Resources.emplace_back();
        sigResource.Name = name;
        sigResource.Type = type;
        sigResource.ShaderRegister = Utils::IsSIGResourceReadOnly(type) ? m_NumSRVs : m_NumUAVs;
        sigResource.ReturnType = returnType;
        sigResource.ArraySize = arraySize;

        if (Utils::IsSIGResourceReadOnly(type))
            m_NumSRVs += arraySize;
        else
            m_NumUAVs += arraySize;

        m_ExistingNames.insert(name);
        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool SIGDeclaration::AddSampler(const std::string& name, uint32_t arraySize)
    {
        if (m_ExistingNames.find(name) != m_ExistingNames.end())
            return false;

        SIGSampler& sigSampler = m_Samplers.emplace_back();
        sigSampler.Name = name;
        sigSampler.ShaderRegister = m_NumSamplers;
        sigSampler.ArraySize = arraySize;

        m_NumSamplers += arraySize;
        m_ExistingNames.insert(name);
        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SIGLayoutDeclaration::SIGLayoutDeclaration(const std::string& name)
        : m_Name(name)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool SIGLayoutDeclaration::AddBindPoint(const std::string& bindPointFullName)
    {
        if (std::find(m_BindPointFullNames.begin(), m_BindPointFullNames.end(), bindPointFullName) != m_BindPointFullNames.end())
            return false;

        m_BindPointFullNames.push_back(bindPointFullName);
        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool SIGLayoutDeclaration::AddStaticSampler(const std::string& staticSamplerFullName)
    {
        if (std::find(m_StaticSamplerFullNames.begin(), m_StaticSamplerFullNames.end(), staticSamplerFullName) != m_StaticSamplerFullNames.end())
            return false;

        m_StaticSamplerFullNames.push_back(staticSamplerFullName);
        return true;
    }
}
