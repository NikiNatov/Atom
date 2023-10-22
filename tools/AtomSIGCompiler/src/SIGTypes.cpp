#include "SIGTypes.h"
#include "SIGUtils.h"

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
    SIGDeclaration::SIGDeclaration(const std::string& name, SIGBindPoint bindPoint)
        : m_Name(name), m_BindPoint(bindPoint)
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

        m_ExistingNames.insert(name);
        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool SIGDeclaration::AddResource(const std::string& name, SIGResourceType type, const std::variant<SIGConstantType, SIGStructType>& returnType)
    {
        if (m_ExistingNames.find(name) != m_ExistingNames.end())
            return false;

        SIGResource& sigResource = m_Resources.emplace_back();
        sigResource.Name = name;
        sigResource.Type = type;
        sigResource.ShaderRegister = Utils::IsSIGResourceReadOnly(type) ? m_NumSRVs++ : m_NumUAVs++;
        sigResource.ReturnType = returnType;

        m_ExistingNames.insert(name);
        return true;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool SIGDeclaration::AddSampler(const std::string& name)
    {
        if (m_ExistingNames.find(name) != m_ExistingNames.end())
            return false;

        SIGSampler& sigSampler = m_Samplers.emplace_back();
        sigSampler.Name = name;
        sigSampler.ShaderRegister = m_NumSamplers++;

        m_ExistingNames.insert(name);
        return true;
    }
}
