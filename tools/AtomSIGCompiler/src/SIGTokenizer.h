#pragma once

#include <string>

namespace SIGCompiler
{
    enum class SIGTokenType
    {
        None = 0,
        Unknown,
        ShaderInputGroup,
        BindPoint,
        BaseType,
        Matrix,
        Struct,
        ResourceType,
        Sampler,
        Identifier,
        LeftAngleBracket,
        RightAngleBracket,
        LeftCurlyBracket,
        RightCurlyBracket,
        SemiColon,
    };

    struct SIGToken
    {
        SIGTokenType Type = SIGTokenType::None;
        std::string Value = "";

        operator bool() const { return Type != SIGTokenType::None; }
    };

    class SIGTokenizer
    {
    public:
        SIGTokenizer(const std::string& sigString);

        const SIGToken& GetCurrentToken() const;
        const SIGToken& GetAndValidateCurrentToken(SIGTokenType expectedType, const char* errorMsg) const;

        SIGTokenizer& operator++();
        SIGTokenizer operator++(int);
    private:
        std::string m_SIGString;
        SIGToken m_CurrentToken;
    };
}