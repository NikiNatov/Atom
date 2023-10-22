#include "SIGTokenizer.h"

#include <vector>
#include <regex>
#include <iostream>

namespace SIGCompiler
{
    static std::vector<std::pair<SIGTokenType, std::regex>> s_Rules = {
        { SIGTokenType::None,              std::regex("[ \n\t\r]+") },
        { SIGTokenType::ShaderInputGroup,  std::regex("ShaderInputGroup") },
        { SIGTokenType::BindPoint,         std::regex("BindPoint::\(Frame|Pass|Material|Instance\)") },
        { SIGTokenType::BaseType,          std::regex("int4|int3|int2|int|uint4|uint3|uint2|uint|float4|float3|float2|float|bool") },
        { SIGTokenType::Matrix,            std::regex("matrix") },
        { SIGTokenType::Struct,            std::regex("struct") },
        { SIGTokenType::ResourceType,      std::regex("Texture2DArray|Texture2D|Texture3D|TextureCube|StructuredBuffer|RWTexture2DArray|RWTexture2D|RWTexture3D|RWStructuredBuffer") },
        { SIGTokenType::Sampler,           std::regex("SamplerState") },
        { SIGTokenType::Identifier,        std::regex("[a-zA-Z_][a-zA-Z_0-9]*") },
        { SIGTokenType::LeftAngleBracket,  std::regex("<") },
        { SIGTokenType::RightAngleBracket, std::regex(">") },
        { SIGTokenType::LeftCurlyBracket,  std::regex("\\{") },
        { SIGTokenType::RightCurlyBracket, std::regex("\\}") },
        { SIGTokenType::SemiColon,         std::regex(";") }
    };

    // -----------------------------------------------------------------------------------------------------------------------------
    SIGTokenizer::SIGTokenizer(const std::string& sigString)
        : m_SIGString(sigString)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const SIGToken& SIGTokenizer::GetCurrentToken() const
    {
        return m_CurrentToken;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    const SIGToken& SIGTokenizer::GetAndValidateCurrentToken(SIGTokenType expectedType, const char* errorMsg) const
    {
        if (m_CurrentToken.Type != expectedType)
            throw std::exception(errorMsg);

        return m_CurrentToken;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SIGTokenizer& SIGTokenizer::operator++()
    {
        if (m_SIGString.empty())
        {
            m_CurrentToken = {}; // Invalid token
            return *this;
        }

        for (auto& [tokenType, tokenRule] : s_Rules)
        {
            std::smatch match;
            if (std::regex_search(m_SIGString, match, tokenRule, std::regex_constants::match_continuous))
            {
                if (tokenType == SIGTokenType::None)
                {
                    // Skip white spaces and new lines
                    m_SIGString = match.suffix();
                    return operator++();
                }

                m_CurrentToken = { tokenType, match.str() };
                m_SIGString = match.suffix();
                return *this;
            }
        }

        m_CurrentToken = { SIGTokenType::Unknown, "" };
        return *this;
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    SIGTokenizer SIGTokenizer::operator++(int)
    {
        SIGTokenizer temp = *this;
        ++(*this);
        return temp;
    }
}
