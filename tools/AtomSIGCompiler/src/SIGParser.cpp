#include "SIGParser.h"
#include "SIGTokenizer.h"
#include "SIGUtils.h"

#include <sstream>
#include <regex>
#include <iostream>
#include <fstream>
#include <format>

namespace SIGCompiler
{
    // -----------------------------------------------------------------------------------------------------------------------------
    SIGParser::SIGParser(const std::filesystem::path& inputDirectory, const std::filesystem::path& outputDirectory)
        : m_InputDirectory(inputDirectory), m_OutputDirectory(outputDirectory)
    {
        std::filesystem::remove_all(m_OutputDirectory);
        std::filesystem::create_directories(m_OutputDirectory / "cpp");
        std::filesystem::create_directories(m_OutputDirectory / "hlsl");
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SIGParser::Parse()
    {
        if (!std::filesystem::exists(m_InputDirectory))
            throw std::exception(std::format("Directory {} does not exist", m_InputDirectory.string().c_str()).c_str());

        // 1. Parse all .sig files in the input directory recursively
        for (std::filesystem::directory_entry entry : std::filesystem::recursive_directory_iterator(m_InputDirectory))
        {
            std::filesystem::path sigFilepath = entry.path();

            if (sigFilepath.extension() != ".sig")
                continue;

            ParseSIGFile(sigFilepath);
        }

        // 2. Validate that all used structs, bind points and static samplers exist
        for (const SIGDeclaration& sig : m_SIGDeclarations)
        {
            std::string bindPointFullName = sig.GetBindPointFullName();

            if (m_BindPointFullNameToID.find(bindPointFullName) == m_BindPointFullNameToID.end())
                throw std::exception(std::format("Unresolved symbol: Bind point \"{}\" does not exist", bindPointFullName).c_str());

            for (const SIGResource& sigResource : sig.GetResources())
            {
                if (const std::string* structTypeString = std::get_if<std::string>(&sigResource.ReturnType))
                {
                    bool found = false;

                    for (const SIGStructType& sigStruct : m_SIGStructs)
                    {
                        if (sigStruct.GetName() == *structTypeString)
                        {
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                        throw std::exception(std::format("Unresolved symbol: Struct \"{}\" does not exist", *structTypeString).c_str());
                }
            }
        }

        // 3. Get all resource counts for each bind point based on the SIGs bound to them
        for (const SIGDeclaration& sig : m_SIGDeclarations)
        {
            uint32_t bpID = m_BindPointFullNameToID.at(sig.GetBindPointFullName());
            m_SIGBindPoints[bpID].ConstantBufferSize += sig.GetConstantBufferSize();
            m_SIGBindPoints[bpID].NumSRVs += sig.GetNumSRVs();
            m_SIGBindPoints[bpID].NumUAVs += sig.GetNumUAVs();
            m_SIGBindPoints[bpID].NumSamplers += sig.GetNumSamplers();
        }

        // 4. Calculate the root parameter indices and the shader space for each bind point and static sampler
        for (SIGLayoutDeclaration& layout : m_SIGLayouts)
        {
            uint32_t currentShaderSpace = 0;
            uint32_t currentRootParamIdx = 0;

            for (const std::string& bindPointFullName : layout.GetBindPointFullNames())
            {
                uint32_t bpID = m_BindPointFullNameToID.at(bindPointFullName);
                m_SIGBindPoints[bpID].ShaderSpace = currentShaderSpace++;

                if (m_SIGBindPoints[bpID].ConstantBufferSize > 0)
                {
                    m_SIGBindPoints[bpID].ConstantBufferRootIdx = currentRootParamIdx++;
                }

                if (m_SIGBindPoints[bpID].NumSRVs + m_SIGBindPoints[bpID].NumUAVs > 0)
                {
                    m_SIGBindPoints[bpID].ResourceTableRootIdx = currentRootParamIdx++;
                }

                if (m_SIGBindPoints[bpID].NumSamplers > 0)
                {
                    m_SIGBindPoints[bpID].SamplerTableRootIdx = currentRootParamIdx++;
                }
            }

            for (const std::string& staticSamplerFullName : layout.GetStaticSamplerFullNames())
            {
                uint32_t samplerID = m_StaticSamplerFullNameToID.at(staticSamplerFullName);
                m_SIGStaticSamplers[samplerID].ShaderSpace = currentShaderSpace;
            }
        }

        // 5. Generate files for all parsed structures
        for (const SIGLayoutDeclaration& layout : m_SIGLayouts)
        {
            GenerateCppFile(layout);
            GenerateHlslFile(layout);
        }

        for (const SIGStructType& sigStruct : m_SIGStructs)
        {
            GenerateHlslFile(sigStruct);
        }

        for (const SIGDeclaration& sig : m_SIGDeclarations)
        {
            GenerateCppFile(sig);
            GenerateHlslFile(sig);
        }

        GenerateSIGDataBaseFile();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SIGParser::ParseSIGFile(const std::filesystem::path& sigFilepath)
    {
        std::ifstream ifs(sigFilepath, std::ios::in);

        if (!ifs.is_open())
            throw std::exception(std::format("SIG file {} could not be opened", sigFilepath.string().c_str()).c_str());

        // Read file
        std::stringstream fileStringStream;
        fileStringStream << ifs.rdbuf();

        // Parse
        SIGTokenizer tokenizer(fileStringStream.str());
        while (SIGToken token = (++tokenizer).GetCurrentToken())
        {
            if (token.Type == SIGTokenType::ShaderInputGroup)
            {
                if (m_CurrentSIGDeclaration || m_CurrentSIGStruct || m_CurrentSIGLayout)
                    throw std::exception("Syntax error: Missing \"}\"");

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected shader input group identifier");

                std::string sigName = token.Value;

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::LeftAngleBracket, "Syntax error: Expected a \"<\"");
                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::BindToAnotation, "Syntax error: Expected target bind point annotation");
                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected bind point identifier");

                std::string layoutName = token.Value;

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Colon, "Syntax error: Expected bind point indentifier");
                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Colon, "Syntax error: Expected bind point indentifier");
                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected bind point indentifier");

                std::string bindPointName = token.Value;

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::RightAngleBracket, "Syntax error: Missing \">\"");

                if (m_ExistingNames.find(sigName) != m_ExistingNames.end())
                    throw std::exception(std::format("Unresolved symbol error: Symbol with name \"{}\" already exists", sigName).c_str());

                m_SIGDeclarations.emplace_back(sigName, layoutName + "::" + bindPointName);
                m_CurrentSIGDeclaration = &m_SIGDeclarations.back();
                m_ExistingNames.insert(sigName);

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::LeftCurlyBracket, "Syntax error: Expected a \"{\"");
            }
            else if (token.Type == SIGTokenType::Struct)
            {
                if (m_CurrentSIGDeclaration || m_CurrentSIGStruct || m_CurrentSIGLayout)
                    throw std::exception("Syntax error: Missing \"}\"");

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected struct identifier");

                std::string sigStructName = token.Value;

                if (m_ExistingNames.find(sigStructName) != m_ExistingNames.end())
                    throw std::exception(std::format("Unresolved symbol error: Symbol with name \"{}\" already exists", sigStructName).c_str());

                m_SIGStructs.emplace_back(sigStructName);
                m_CurrentSIGStruct = &m_SIGStructs.back();
                m_ExistingNames.insert(sigStructName);

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::LeftCurlyBracket, "Syntax error: Expected a \"{\"");
            }
            else if (token.Type == SIGTokenType::ShaderInputLayout)
            {
                if (m_CurrentSIGDeclaration || m_CurrentSIGStruct || m_CurrentSIGLayout)
                    throw std::exception("Syntax error: Missing \"}\"");

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected shader input layout identifier");

                std::string sigLayoutName = token.Value;

                if (m_ExistingNames.find(sigLayoutName) != m_ExistingNames.end())
                    throw std::exception(std::format("Unresolved symbol error: Symbol with name \"{}\" already exists", sigLayoutName).c_str());

                m_SIGLayouts.emplace_back(sigLayoutName);
                m_CurrentSIGLayout = &m_SIGLayouts.back();
                m_ExistingNames.insert(sigLayoutName);

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::LeftCurlyBracket, "Syntax error: Expected a \"{\"");
            }
            else if (token.Type == SIGTokenType::BindPoint)
            {
                if (!m_CurrentSIGLayout)
                    throw std::exception("Syntax error: BindPoint declarations are only allowed within shader input layouts");

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected bind point identifier");

                std::string bindPointName = token.Value;

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::SemiColon, "Syntax error: Expected \";\"");

                SIGBindPoint& bindPoint = m_SIGBindPoints.emplace_back();
                bindPoint.Name = bindPointName;
                bindPoint.LayoutName = m_CurrentSIGLayout->GetName();
                bindPoint.FullName = bindPoint.LayoutName + "::" + bindPoint.Name;

                if(!m_CurrentSIGLayout->AddBindPoint(bindPoint.FullName))
                    throw std::exception(std::format("Unresolved symbol error: Bind point redefinition \"{}\"", bindPoint.FullName).c_str());

                m_BindPointFullNameToID[bindPoint.FullName] = m_SIGBindPoints.size() - 1;
            }
            else if (token.Type == SIGTokenType::StaticSampler)
            {
                if (!m_CurrentSIGLayout)
                    throw std::exception("Syntax error: StaticSampler declarations are only allowed within shader input layouts");

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected static sampler identifier");

                std::string staticSamplerName = token.Value;

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Equals, "Syntax error: Expected \"=\"");
                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::LeftCurlyBracket, "Syntax error: Expected \"{\"");
                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::SamplerFilter, "Syntax error: Expected static sampler filter");

                SamplerFilter filter = Utils::StringToSamplerFilter(token.Value);

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Comma, "Syntax error: Expected \",\"");
                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::SamplerWrap, "Syntax error: Expected static sampler wrap");

                SamplerWrap wrap = Utils::StringToSamplerWrap(token.Value);

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::RightCurlyBracket, "Syntax error: Expected \"}\"");
                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::SemiColon, "Syntax error: Expected \";\"");

                SIGStaticSampler& staticSampler = m_SIGStaticSamplers.emplace_back();
                staticSampler.Name = staticSamplerName;
                staticSampler.LayoutName = m_CurrentSIGLayout->GetName();
                staticSampler.FullName = staticSampler.LayoutName + "::" + staticSampler.Name;
                staticSampler.ShaderRegister = m_CurrentSIGLayout->GetStaticSamplerFullNames().size();
                staticSampler.Filter = filter;
                staticSampler.Wrap = wrap;

                if (!m_CurrentSIGLayout->AddStaticSampler(staticSampler.FullName))
                    throw std::exception(std::format("Unresolved symbol error: Static sampler redefinition \"{}\"", staticSampler.FullName).c_str());

                m_StaticSamplerFullNameToID[staticSampler.FullName] = m_SIGStaticSamplers.size() - 1;
            }
            else if (token.Type == SIGTokenType::BaseType || token.Type == SIGTokenType::Matrix)
            {
                if (!m_CurrentSIGDeclaration && !m_CurrentSIGStruct)
                    throw std::exception("Syntax error: Variable declarations are only allowed within shader input groups and structs");

                SIGConstantType sigConstantType = Utils::StringToSIGConstantType(token.Value);

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected shader constant identifier");

                std::string sigConstantName = token.Value;

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::SemiColon, "Syntax error: Expected a \";\"");

                if (m_CurrentSIGDeclaration)
                {
                    if (!m_CurrentSIGDeclaration->AddConstant(sigConstantName, sigConstantType))
                        throw std::exception(std::format("Syntax error: Constant name redefinition \"{}\"", sigConstantName.c_str()).c_str());
                }
                else
                {
                    if (!m_CurrentSIGStruct->AddMember(sigConstantName, sigConstantType))
                        throw std::exception(std::format("Syntax error: Struct member name redefinition \"{}\"", sigConstantName.c_str()).c_str());
                }
            }
            else if (token.Type == SIGTokenType::ResourceType)
            {
                if (!m_CurrentSIGDeclaration)
                    throw std::exception("Syntax error: Resource declarations are only allowed within shader input groups");

                SIGResourceType sigResourceType = Utils::StringToSIGResourceType(token.Value);

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::LeftAngleBracket, "Syntax error: Expected \"<\"");

                // Use float4 as default return type
                std::variant<SIGConstantType, std::string> sigResourceReturnType = SIGConstantType::Float4;

                token = (++tokenizer).GetCurrentToken();

                if (token.Type == SIGTokenType::BaseType)
                {
                    sigResourceReturnType = Utils::StringToSIGConstantType(token.Value);
                }
                else if (token.Type == SIGTokenType::Matrix)
                {
                    if (sigResourceType != SIGResourceType::StructuredBuffer && sigResourceType != SIGResourceType::RWStructuredBuffer)
                        throw std::exception("Syntax error: Texture resources cannot have matrix as a return type");

                    sigResourceReturnType = Utils::StringToSIGConstantType(token.Value);
                }
                else if (token.Type == SIGTokenType::Identifier)
                {
                    if (sigResourceType != SIGResourceType::StructuredBuffer && sigResourceType != SIGResourceType::RWStructuredBuffer)
                        throw std::exception("Syntax error: Texture resources cannot have struct as return type");

                    // For custom struct types store the name of the type and resolve it later
                    sigResourceReturnType = token.Value;
                }
                else
                    throw std::exception("Syntax error: Expected type identifier");

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::RightAngleBracket, "Syntax error: Missing \">\"");
                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected resource identifier");

                std::string sigResourceName = token.Value;

                // Check for array
                token = (++tokenizer).GetCurrentToken();

                if (token.Type == SIGTokenType::LeftSquareBracket)
                {
                    token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Number, "Syntax error: Expected array size value");

                    int arraySize = atoi(token.Value.c_str());

                    if (arraySize == 0)
                        throw std::exception("Syntax error: Array size cannot be 0");

                    token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::RightSquareBracket, "Syntax error: Expected a \"]\"");
                    token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::SemiColon, "Syntax error: Expected a \";\"");

                    if (!m_CurrentSIGDeclaration->AddResource(sigResourceName, sigResourceType, sigResourceReturnType, arraySize))
                        throw std::exception(std::format("Unresolved symbol error: Resource name redefinition \"{}\"", sigResourceName.c_str()).c_str());
                }
                else if (token.Type == SIGTokenType::SemiColon)
                {
                    if (!m_CurrentSIGDeclaration->AddResource(sigResourceName, sigResourceType, sigResourceReturnType))
                        throw std::exception(std::format("Unresolved symbol error: Resource name redefinition \"{}\"", sigResourceName.c_str()).c_str());
                }
                else
                    throw std::exception("Syntax error: Expected a \";\"");
            }
            else if (token.Type == SIGTokenType::Sampler)
            {
                if (!m_CurrentSIGDeclaration)
                    throw std::exception("Syntax error: Sampler declarations are only allowed within shader input groups");

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected sampler identifier");

                std::string sigSamplerName = token.Value;

                token = (++tokenizer).GetCurrentToken();

                uint32_t arraySize = 1;
                if (token.Type == SIGTokenType::LeftSquareBracket)
                {
                    // Check for array
                    token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Number, "Syntax error: Expected array size value");

                    arraySize = atoi(token.Value.c_str());

                    if (arraySize == 0)
                        throw std::exception("Syntax error: Array size cannot be 0");

                    token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::RightSquareBracket, "Syntax error: Expected a \"]\"");
                    token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::SemiColon, "Syntax error: Expected a \";\"");
                }
                else if (token.Type != SIGTokenType::SemiColon)
                    throw std::exception(std::format("Syntax error: Unexpected token \"{}\"", token.Value).c_str());

                if (!m_CurrentSIGDeclaration->AddSampler(sigSamplerName, arraySize))
                    throw std::exception(std::format("Unresolved symbol error: Sampler name redefinition \"{}\"", sigSamplerName.c_str()).c_str());
            }
            else if (token.Type == SIGTokenType::RightCurlyBracket)
            {
                if (!m_CurrentSIGDeclaration && !m_CurrentSIGStruct && !m_CurrentSIGLayout)
                    throw std::exception("Syntax error: Unexpected \"}\"");

                m_CurrentSIGDeclaration = nullptr;
                m_CurrentSIGStruct = nullptr;
                m_CurrentSIGLayout = nullptr;
            }
            else if (token.Type == SIGTokenType::SemiColon)
            {
                continue;
            }
            else
            {
                throw std::exception("Syntax error: Unexpected token");
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SIGParser::GenerateSIGDataBaseFile()
    {
        std::ostringstream ss;

        ss << "#pragma once\n\n";
        ss << "#include \"Atom/Core/Core.h\"\n";

        for (const SIGLayoutDeclaration& layout : m_SIGLayouts)
        {
            ss << "#include \"" << layout.GetName() << ".h\"\n";
        }

        ss << "\n";
        ss << "namespace Atom { namespace SIG {\n";

        ss << "\tclass SIGDataBase\n";
        ss << "\t{\n";
        ss << "\tpublic:\n";

        ss << "\t\tstatic void Initialize()\n";
        ss << "\t\t{\n";

        for (const SIGLayoutDeclaration& layout : m_SIGLayouts)
        {
            ss << "\t\t\t" << layout.GetName() << "::Initialize();\n";
        }

        ss << "\t\t}\n\n";

        ss << "\t\tstatic void Shutdown()\n";
        ss << "\t\t{\n";

        for (const SIGLayoutDeclaration& layout : m_SIGLayouts)
        {
            ss << "\t\t\t" << layout.GetName() << "::Destroy();\n";
        }

        ss << "\t\t}\n\n";

        ss << "\t};\n";

        ss << "}}\n";

        // Create file on disk
        std::string outputPath = m_OutputDirectory.string() + "/cpp/SIGDataBase.h";
        std::ofstream ofs(outputPath, std::ios::out | std::ios::trunc);

        if (!ofs.is_open())
            throw std::exception(std::format("File {} could not be created", outputPath.c_str()).c_str());

        ofs << ss.str();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SIGParser::GenerateCppFile(const SIGLayoutDeclaration& sigLayout)
    {
        std::ostringstream ss;

        ss << "#pragma once\n\n";
        ss << "#include \"Atom/Core/Core.h\"\n";
        ss << "#include \"Atom/Renderer/SIG/ShaderInputGroupLayout.h\"\n\n";

        ss << "namespace Atom { namespace SIG {\n";
        ss << "\tclass " << sigLayout.GetName() << " : public ShaderInputGroupLayout\n";
        ss << "\t{\n";
        ss << "\tpublic:\n";

        uint32_t currentRootParamIdx = 0;

        for (const std::string& bindPointFullName : sigLayout.GetBindPointFullNames())
        {
            uint32_t bpID = m_BindPointFullNameToID.at(bindPointFullName);

            ss << "\t\tstruct " << m_SIGBindPoints[bpID].Name << " : public ShaderInputGroupBindPoint\n";
            ss << "\t\t{\n";
            ss << "\t\t\ttypedef " << sigLayout.GetName() << " LayoutType;\n\n";
            ss << "\t\t\tenum\n";
            ss << "\t\t\t{\n";
            ss << "\t\t\t\tShaderSpace = " << m_SIGBindPoints[bpID].ShaderSpace << ",\n";
            ss << "\t\t\t\tConstantBufferSize = " << m_SIGBindPoints[bpID].ConstantBufferSize << ",\n";
            ss << "\t\t\t\tNumSRVs = " << m_SIGBindPoints[bpID].NumSRVs << ",\n";
            ss << "\t\t\t\tNumUAVs = " << m_SIGBindPoints[bpID].NumUAVs << ",\n";
            ss << "\t\t\t\tNumSamplers = " << m_SIGBindPoints[bpID].NumSamplers << ",\n";
            ss << "\t\t\t\tConstantBufferRootIndex = " << m_SIGBindPoints[bpID].ConstantBufferRootIdx << ",\n";
            ss << "\t\t\t\tResourceTableRootIndex = " << m_SIGBindPoints[bpID].ResourceTableRootIdx << ",\n";
            ss << "\t\t\t\tSamplerTableRootIndex = " << m_SIGBindPoints[bpID].SamplerTableRootIdx << "\n";
            ss << "\t\t\t};\n\n";
            ss << "\t\t\t" << m_SIGBindPoints[bpID].Name << "()";
            ss << " : ShaderInputGroupBindPoint(ShaderInputGroupBindPoint::Description::Create<" << m_SIGBindPoints[bpID].Name << ">()) {}\n";
            ss << "\t\t};\n\n";
        }

        for (const std::string& staticSamplerFullName : sigLayout.GetStaticSamplerFullNames())
        {
            uint32_t samplerID = m_StaticSamplerFullNameToID.at(staticSamplerFullName);
            const SIGStaticSampler& sampler = m_SIGStaticSamplers[samplerID];

            ss << "\t\tstruct " << sampler.Name << " : public ShaderInputGroupStaticSampler\n";
            ss << "\t\t{\n";
            ss << "\t\t\ttypedef " << sigLayout.GetName() << " LayoutType;\n\n";
            ss << "\t\t\tenum\n";
            ss << "\t\t\t{\n";
            ss << "\t\t\t\tShaderSpace = " << sampler.ShaderSpace << ",\n";
            ss << "\t\t\t};\n\n";
            ss << "\t\t\t" << sampler.Name << "()";
            ss << " : ShaderInputGroupStaticSampler(";
            ss << sampler.Name << "::ShaderSpace, ";
            ss << Utils::SamplerFilterToCppString(sampler.Filter) << ",";
            ss << Utils::SamplerWrapToCppString(sampler.Wrap) << ") {}\n";
            ss << "\t\t};\n\n";
        }

        ss << "\tpublic:\n";
        ss << "\t\tinline static void Initialize() { ATOM_ENGINE_ASSERT(ms_Instance == nullptr); ms_Instance = new " << sigLayout.GetName() << "(); }\n";
        ss << "\t\tinline static void Destroy() { delete ms_Instance; }\n";
        ss << "\t\tinline static " << sigLayout.GetName() << "* GetInstance() { ATOM_ENGINE_ASSERT(ms_Instance, \"Layout not initialized\"); return ms_Instance; }\n";

        ss << "\tprivate:\n";
        ss << "\t\t" << sigLayout.GetName() << "() : ShaderInputGroupLayout({ ";

        for (uint32_t i = 0; i < sigLayout.GetBindPointFullNames().size(); i++)
        {
            if (i != 0)
                ss << ", ";

            uint32_t bpID = m_BindPointFullNameToID.at(sigLayout.GetBindPointFullNames()[i]);
            ss << "&ms_" << m_SIGBindPoints[bpID].Name << "BindPoint";
        }

        ss << " }, { ";
        
        for (uint32_t i = 0; i < sigLayout.GetStaticSamplerFullNames().size(); i++)
        {
            if (i != 0)
                ss << ", ";

            uint32_t samplerID = m_StaticSamplerFullNameToID.at(sigLayout.GetStaticSamplerFullNames()[i]);
            ss << "&ms_" << m_SIGStaticSamplers[samplerID].Name;
        }

        ss << "}) {}\n";

        ss << "\tprivate:\n";

        for (const std::string& bindPointFullName : sigLayout.GetBindPointFullNames())
        {
            uint32_t bpID = m_BindPointFullNameToID.at(bindPointFullName);
            ss << "\t\tinline static const " << m_SIGBindPoints[bpID].Name << " ms_" << m_SIGBindPoints[bpID].Name << "BindPoint;\n";
        }

        for (const std::string& staticSamplerFullName : sigLayout.GetStaticSamplerFullNames())
        {
            uint32_t samplerID = m_StaticSamplerFullNameToID.at(staticSamplerFullName);
            ss << "\t\tinline static const " << m_SIGStaticSamplers[samplerID].Name << " ms_" << m_SIGStaticSamplers[samplerID].Name << ";\n";
        }

        ss << "\tprivate:\n";
        ss << "\t\tinline static " << sigLayout.GetName() << "* ms_Instance = nullptr;\n";
        ss << "\t};\n";
        ss << "}}\n";

        // Create file on disk
        std::string outputPath = m_OutputDirectory.string() + "/cpp/" + sigLayout.GetName() + ".h";
        std::ofstream ofs(outputPath, std::ios::out | std::ios::trunc);

        if (!ofs.is_open())
            throw std::exception(std::format("File {} could not be created", outputPath.c_str()).c_str());

        ofs << ss.str();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SIGParser::GenerateHlslFile(const SIGLayoutDeclaration& sigLayout)
    {
        std::ostringstream ss;

        ss << "#ifndef __" << sigLayout.GetName() << "_HLSLI__\n";
        ss << "#define __" << sigLayout.GetName() << "_HLSLI__\n\n";

        if (!sigLayout.GetStaticSamplerFullNames().empty())
        {
            // Write static samplers struct
            std::string structName = sigLayout.GetName() + "StaticSamplers";
            ss << "struct " << structName << "\n";
            ss << "{\n";

            for (const std::string& staticSamplerFullName : sigLayout.GetStaticSamplerFullNames())
            {
                uint32_t samplerID = m_StaticSamplerFullNameToID.at(staticSamplerFullName);
                ss << "\tSamplerState " << m_SIGStaticSamplers[samplerID].Name << ";\n";
            }

            ss << "};\n\n";

            for (const std::string& staticSamplerFullName : sigLayout.GetStaticSamplerFullNames())
            {
                uint32_t samplerID = m_StaticSamplerFullNameToID.at(staticSamplerFullName);
                const SIGStaticSampler& sampler = m_SIGStaticSamplers[samplerID];
                ss << "SamplerState " << sigLayout.GetName() << "_" << sampler.Name << " : register(s" << sampler.ShaderRegister << ", space" << sampler.ShaderSpace << ");\n";
            }

            ss << "\n";

            ss << structName << " Create" << structName << "()\n";
            ss << "{\n";
            ss << "\t" << structName << " resources;\n";

            for (const std::string& staticSamplerFullName : sigLayout.GetStaticSamplerFullNames())
            {
                uint32_t samplerID = m_StaticSamplerFullNameToID.at(staticSamplerFullName);
                ss << "\tresources." << m_SIGStaticSamplers[samplerID].Name << " = " << sigLayout.GetName() << "_" << m_SIGStaticSamplers[samplerID].Name << ";\n";
            }

            ss << "\treturn resources;\n";
            ss << "}\n\n";
        }

        auto GenerateRootSignature = [&](bool isCompute)
        {
            ss << "#define " << sigLayout.GetName() << "_" << (isCompute ? "Compute" : "Graphics") << " \\\n";

            if (!isCompute)
            {
                ss << "\"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT),\" \\\n";
            }
            else
            {
                ss << "\"RootFlags(DENY_VERTEX_SHADER_ROOT_ACCESS | ";
                ss << "DENY_HULL_SHADER_ROOT_ACCESS | ";
                ss << "DENY_DOMAIN_SHADER_ROOT_ACCESS | ";
                ss << "DENY_GEOMETRY_SHADER_ROOT_ACCESS | ";
                ss << "DENY_PIXEL_SHADER_ROOT_ACCESS),\" \\\n";
            }

            for (uint32_t i = 0; i < sigLayout.GetBindPointFullNames().size(); i++)
            {
                uint32_t bpID = m_BindPointFullNameToID.at(sigLayout.GetBindPointFullNames()[i]);
                const SIGBindPoint& bp = m_SIGBindPoints[bpID];

                if (bp.ConstantBufferRootIdx != UINT32_MAX)
                {
                    ss << "\"CBV(b0, space = " << bp.ShaderSpace << ")";
                    ss << ",\"" << " \\\n";
                }

                if (bp.ResourceTableRootIdx != UINT32_MAX)
                {
                    ss << "\"DescriptorTable(";

                    if (bp.NumSRVs > 0)
                    {
                        ss << "SRV(t0, space = " << bp.ShaderSpace << ", numDescriptors = " << bp.NumSRVs << ", flags = DESCRIPTORS_VOLATILE)";

                        if (bp.NumUAVs > 0)
                            ss << ", ";
                    }

                    if (bp.NumUAVs > 0)
                    {
                        ss << "UAV(u0, space = " << bp.ShaderSpace << ", numDescriptors = " << bp.NumUAVs << ", flags = DESCRIPTORS_VOLATILE)";
                    }

                    ss << "),\"" << " \\\n";
                }

                if (bp.SamplerTableRootIdx != UINT32_MAX)
                {
                    ss << "\"DescriptorTable(Sampler(s0, space = " << bp.ShaderSpace << ", numDescriptors = " << bp.NumSamplers << ", flags = DESCRIPTORS_VOLATILE))";
                    ss << ",\"" << " \\\n";
                }
            }

            for (uint32_t i = 0; i < sigLayout.GetStaticSamplerFullNames().size(); i++)
            {
                uint32_t samplerID = m_StaticSamplerFullNameToID.at(sigLayout.GetStaticSamplerFullNames()[i]);
                const SIGStaticSampler& sampler = m_SIGStaticSamplers[samplerID];

                std::string filter = Utils::SamplerFilterToHlslString(sampler.Filter);
                std::string wrap = Utils::SamplerWrapToHlslString(sampler.Wrap);
                ss << "\"StaticSampler(s" << sampler.ShaderRegister << ", space = " << sampler.ShaderSpace;
                ss << ", filter = " << filter << ", addressU = " << wrap << ", addressV = " << wrap << ", addressW = " << wrap;
                ss << ", maxAnisotropy = " << (sampler.Filter != SamplerFilter::Anisotropic ? 1 : 16);
                ss << "),\"" << "\\\n";
            }

            ss << "\n";
        };

        GenerateRootSignature(false);
        GenerateRootSignature(true);

        ss << "#endif // __" << sigLayout.GetName() << "_HLSLI__\n";

        // Create file on disk
        std::string outputPath = m_OutputDirectory.string() + "/hlsl/" + sigLayout.GetName() + ".hlsli";
        std::ofstream ofs(outputPath, std::ios::out | std::ios::trunc);

        if (!ofs.is_open())
            throw std::exception(std::format("File {} could not be created", outputPath.c_str()).c_str());

        ofs << ss.str();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SIGParser::GenerateCppFile(const SIGDeclaration& sig)
    {
        std::ostringstream ss;

        uint32_t bpID = m_BindPointFullNameToID.at(sig.GetBindPointFullName());
        const SIGBindPoint& bindPoint = m_SIGBindPoints[bpID];

        ss << "#pragma once\n\n";
        ss << "#include \"Atom/Core/Core.h\"\n";
        ss << "#include \"Atom/Core/DirectX12/DirectX12.h\"\n";
        ss << "#include \"Atom/Renderer/Texture.h\"\n";
        ss << "#include \"Atom/Renderer/Buffer.h\"\n";
        ss << "#include \"Atom/Renderer/SIG/ShaderInputGroup.h\"\n";
        ss << "#include \"Atom/Renderer/SIG/ShaderInputGroupStorage.h\"\n";
        ss << "#include \"" << bindPoint.LayoutName << ".h\"\n";
        ss << "#include <glm/glm.hpp>\n\n";

        ss << "namespace Atom { namespace SIG {\n";

        ss << "\tclass " << sig.GetName() << " : public ShaderInputGroup\n";
        ss << "\t{\n";
        ss << "\tpublic:\n";
        ss << "\t\ttypedef " << bindPoint.FullName << " BindPointType;\n\n";

        // Write constants struct
        ss << "\t\tstruct Constants\n";
        ss << "\t\t{\n";
        for (const SIGConstant& sigConst : sig.GetConstants())
        {
            ss << "\t\t\t" << Utils::SIGConstantTypeToCppString(sigConst.Type) << " " << sigConst.Name << ";\n";
        }
        ss << "\t\t};\n\n";

        // Write SRV structs
        ss << "\t\tstruct SRV\n";
        ss << "\t\t{\n";
        for (const SIGResource& sigResource : sig.GetResources())
        {
            if (Utils::IsSIGResourceReadOnly(sigResource.Type))
            {
                if (sigResource.ArraySize > 1)
                {
                    for (uint32_t i = 0; i < sigResource.ArraySize; i++)
                    {
                        ss << "\t\t\tD3D12_CPU_DESCRIPTOR_HANDLE " << sigResource.Name << i << ";\n";
                    }
                }
                else
                {
                    ss << "\t\t\tD3D12_CPU_DESCRIPTOR_HANDLE " << sigResource.Name << ";\n";
                }
            }
        }
        ss << "\t\t};\n\n";

        ss << "\t\tstruct SRVForLayout\n";
        ss << "\t\t{\n";
        ss << "\t\t\tunion\n";
        ss << "\t\t\t{\n";
        ss << "\t\t\t\tSRV LocalSRV;\n";
        ss << "\t\t\t\tD3D12_CPU_DESCRIPTOR_HANDLE LayoutSRV[BindPointType::NumSRVs];\n";
        ss << "\t\t\t};\n";
        ss << "\t\t};\n\n";

        // Write UAV structs
        ss << "\t\tstruct UAV\n";
        ss << "\t\t{\n";
        for (const SIGResource& sigResource : sig.GetResources())
        {
            if (!Utils::IsSIGResourceReadOnly(sigResource.Type))
            {
                if (sigResource.ArraySize > 1)
                {
                    for (uint32_t i = 0; i < sigResource.ArraySize; i++)
                    {
                        ss << "\t\t\tD3D12_CPU_DESCRIPTOR_HANDLE " << sigResource.Name << i << ";\n";
                    }
                }
                else
                {
                    ss << "\t\t\tD3D12_CPU_DESCRIPTOR_HANDLE " << sigResource.Name << ";\n";
                }
            }
        }
        ss << "\t\t};\n\n";

        ss << "\t\tstruct UAVForLayout\n";
        ss << "\t\t{\n";
        ss << "\t\t\tunion\n";
        ss << "\t\t\t{\n";
        ss << "\t\t\t\tUAV LocalUAV;\n";
        ss << "\t\t\t\tD3D12_CPU_DESCRIPTOR_HANDLE LayoutUAV[BindPointType::NumUAVs];\n";
        ss << "\t\t\t};\n";
        ss << "\t\t};\n\n";

        // Write Samplers structs
        ss << "\t\tstruct Samplers\n";
        ss << "\t\t{\n";
        for (const SIGSampler& sigSampler : sig.GetSamplers())
        {
            if (sigSampler.ArraySize > 1)
            {
                for (uint32_t i = 0; i < sigSampler.ArraySize; i++)
                {
                    ss << "\t\t\tD3D12_CPU_DESCRIPTOR_HANDLE " << sigSampler.Name << i << ";\n";
                }
            }
            else
            {
                ss << "\t\t\tD3D12_CPU_DESCRIPTOR_HANDLE " << sigSampler.Name << ";\n";
            }
        }
        ss << "\t\t};\n\n";

        ss << "\t\tstruct SamplersForLayout\n";
        ss << "\t\t{\n";
        ss << "\t\t\tunion\n";
        ss << "\t\t\t{\n";
        ss << "\t\t\t\tSamplers LocalSamplers;\n";
        ss << "\t\t\t\tD3D12_CPU_DESCRIPTOR_HANDLE LayoutSamplers[BindPointType::NumSamplers];\n";
        ss << "\t\t\t};\n";
        ss << "\t\t};\n\n";

        // Write constructor
        ss << "\tpublic:\n";
        ss << "\t\t" << sig.GetName() << "(bool persistentStorage = false)\n";
        ss << "\t\t\t: ShaderInputGroup(ShaderInputGroupStorage::Create<" << sig.GetName() << ">(persistentStorage))\n";
        ss << "\t\t{}\n\n";

        // Write constants setters
        for (const SIGConstant& sigConst : sig.GetConstants())
        {
            ss << "\t\tvoid Set" << sigConst.Name << "(const " << Utils::SIGConstantTypeToCppString(sigConst.Type) << "& value)\n";
            ss << "\t\t{\n";
            ss << "\t\t\tConstants* constants = m_SIGStorage.GetConstants<Constants>();\n";
            ss << "\t\t\tconstants->" << sigConst.Name << " = value;\n";
            ss << "\t\t\tm_SIGStorage.SetConstantsDirtyFlag(true);\n";
            ss << "\t\t}\n\n";
        }

        // Write resources setters
        for (const SIGResource& sigResource : sig.GetResources())
        {
            ss << "\t\tvoid Set" << sigResource.Name;

            if (sigResource.ArraySize > 1)
                ss << "AtIndex(u32 index, ";
            else
                ss << "(";

            ss << "const " << Utils::SIGResourceTypeToCppString(sigResource.Type) << "* value)\n";
            ss << "\t\t{\n";

            if (sigResource.ArraySize > 1)
                ss << "\t\t\tATOM_ENGINE_ASSERT(index < " << sigResource.ArraySize << ");\n";

            if (Utils::IsSIGResourceReadOnly(sigResource.Type))
            {
                ss << "\t\t\tSRVForLayout* srv = m_SIGStorage.GetSRVDescriptors<SRVForLayout>();\n";

                if (sigResource.ArraySize > 1)
                {
                    ss << "\t\t\t*((D3D12_CPU_DESCRIPTOR_HANDLE*)(&srv->LocalSRV." << sigResource.Name << "0) + index) = ";
                }
                else
                {
                    ss << "\t\t\tsrv->LocalSRV." << sigResource.Name << " = ";
                }

                ss << "value->GetSRV()" << (Utils::IsSIGResourceTexture(sigResource.Type) ? "->GetDescriptor();\n" : ";\n");
            }
            else
            {
                ss << "\t\t\tUAVForLayout* uav = m_SIGStorage.GetUAVDescriptors<UAVForLayout>();\n";

                if (sigResource.ArraySize > 1)
                {
                    ss << "\t\t\t*((D3D12_CPU_DESCRIPTOR_HANDLE*)(&uav->LocalUAV." << sigResource.Name << "0) + index) = ";
                }
                else
                {
                    ss << "\t\t\tuav->LocalUAV." << sigResource.Name << " = ";
                }

                ss << "value->GetUAV()" << (Utils::IsSIGResourceTexture(sigResource.Type) ? "->GetDescriptor();\n" : ";\n");
            }

            ss << "\t\t\tm_SIGStorage.SetResourcesDirtyFlag(true);\n";
            ss << "\t\t}\n\n";
        }

        // Write sampler setters
        for (const SIGSampler& sigSampler : sig.GetSamplers())
        {
            ss << "\t\tvoid Set" << sigSampler.Name;

            if (sigSampler.ArraySize > 1)
                ss << "AtIndex(u32 index, ";
            else
                ss << "(";

            ss << "const TextureSampler* value)\n";

            ss << "\t\t{\n";

            if (sigSampler.ArraySize > 1)
            {
                ss << "\t\t\tATOM_ENGINE_ASSERT(index < " << sigSampler.ArraySize << ");\n";
            }

            ss << "\t\t\tSamplersForLayout* smp = m_SIGStorage.GetSamplerDescriptors<SamplersForLayout>();\n";

            if (sigSampler.ArraySize > 1)
            {
                ss << "\t\t\t*((D3D12_CPU_DESCRIPTOR_HANDLE*)(&smp->LocalSamplers." << sigSampler.Name << "0) + index) = ";
            }
            else
            {
                ss << "\t\t\tsmp->LocalSamplers." << sigSampler.Name << " = ";
            }

            ss << "value->GetDescriptor();\n";
            ss << "\t\t\tm_SIGStorage.SetSamplersDirtyFlag(true);\n";
            ss << "\t\t}\n\n";
        }

        ss << "\t};\n\n";

        ss << "}}\n";

        // Create file on disk
        std::string outputPath = m_OutputDirectory.string() + "/cpp/" + sig.GetName() + ".h";
        std::ofstream ofs(outputPath, std::ios::out | std::ios::trunc);

        if (!ofs.is_open())
            throw std::exception(std::format("File {} could not be created", outputPath.c_str()).c_str());

        ofs << ss.str();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SIGParser::GenerateHlslFile(const SIGDeclaration& sig)
    {
        std::ostringstream ss;

        ss << "#ifndef __" << sig.GetName() << "_HLSLI__\n";
        ss << "#define __" << sig.GetName() << "_HLSLI__\n\n";

        for (const SIGResource& sigResource : sig.GetResources())
        {
            if (const std::string* structTypeString = std::get_if<std::string>(&sigResource.ReturnType))
            {
                ss << "#include \"autogen/hlsl/" << *structTypeString << ".hlsli\"\n";
            }
        }

        ss << "\n";

        // Write params struct
        ss << "struct " << sig.GetName() << "\n";
        ss << "{\n";

        for (const SIGConstant& sigConst : sig.GetConstants())
        {
            ss << "\t" << Utils::SIGConstantTypeToHlslString(sigConst.Type) << " " << sigConst.Name << ";\n";
        }

        for (const SIGResource& sigResource : sig.GetResources())
        {
            ss << "\t" << Utils::SIGResourceTypeToHlslString(sigResource.Type) << "<";

            if (const SIGConstantType* constTypePtr = std::get_if<SIGConstantType>(&sigResource.ReturnType))
            {
                ss << Utils::SIGConstantTypeToHlslString(*constTypePtr);
            }
            else if (const std::string* structTypeString = std::get_if<std::string>(&sigResource.ReturnType))
            {
                ss << *structTypeString;
            }

            ss << "> " << sigResource.Name;

            if (sigResource.ArraySize > 1)
                ss << "[" << sigResource.ArraySize << "]";

            ss << ";\n";
        }

        for (const SIGSampler& sigSampler : sig.GetSamplers())
        {
            ss << "\tSamplerState " << sigSampler.Name;

            if (sigSampler.ArraySize > 1)
                ss << "[" << sigSampler.ArraySize << "]";

            ss << ";\n";
        }

        ss << "};\n\n";

        uint32_t bpID = m_BindPointFullNameToID.at(sig.GetBindPointFullName());
        const SIGBindPoint& bp = m_SIGBindPoints[bpID];

        // Write constant buffer declaration
        if (!sig.GetConstants().empty())
        {
            ss << "// -------------------------------------------------- Constants --------------------------------------------------- //\n";
            ss << "cbuffer " << sig.GetName() << "_Constants : register(b0, space" << bp.ShaderSpace << ")\n";
            ss << "{\n";

            for (const SIGConstant& sigConst : sig.GetConstants())
            {
                ss << "\t" << Utils::SIGConstantTypeToHlslString(sigConst.Type) << " " << sig.GetName() << "_" << sigConst.Name << ";\n";
            }

            ss << "};\n\n";
        }

        // Write resource declarations
        if (!sig.GetResources().empty())
        {
            ss << "// -------------------------------------------------- Resources --------------------------------------------------- //\n";

            for (const SIGResource& sigResource : sig.GetResources())
            {
                ss << Utils::SIGResourceTypeToHlslString(sigResource.Type) << "<";

                if (const SIGConstantType* constTypePtr = std::get_if<SIGConstantType>(&sigResource.ReturnType))
                {
                    ss << Utils::SIGConstantTypeToHlslString(*constTypePtr);
                }
                else if (const std::string* structTypeString = std::get_if<std::string>(&sigResource.ReturnType))
                {
                    ss << *structTypeString;
                }

                ss << "> " << sig.GetName() << "_" << sigResource.Name;

                if (sigResource.ArraySize > 1)
                {
                    ss << "[" << sigResource.ArraySize << "]";
                }

                ss << " : register(" << (Utils::IsSIGResourceReadOnly(sigResource.Type) ? "t" : "u") << sigResource.ShaderRegister << ", space" << bp.ShaderSpace << ");\n";
            }

            ss << "\n";
        }

        // Write sampler declarations
        if (!sig.GetSamplers().empty())
        {
            ss << "// -------------------------------------------------- Samplers --------------------------------------------------- //\n";

            for (const SIGSampler& sigSampler : sig.GetSamplers())
            {
                ss << "SamplerState " << sig.GetName() << "_" << sigSampler.Name;

                if (sigSampler.ArraySize > 1)
                {
                    ss << "[" << sigSampler.ArraySize << "]";
                }

                ss << " : register(s" << sigSampler.ShaderRegister << ", space" << bp.ShaderSpace << ");\n";
            }

            ss << "\n";
        }

        // Write params struct creation function
        ss << sig.GetName() << " Create" << sig.GetName() << "()\n";
        ss << "{\n";
        ss << "\t" << sig.GetName() << " resources;\n";

        for (const SIGConstant& sigConst : sig.GetConstants())
        {
            ss << "\tresources." << sigConst.Name << " = " << sig.GetName() << "_" << sigConst.Name << ";\n";
        }

        for (const SIGResource& sigResource : sig.GetResources())
        {
            ss << "\tresources." << sigResource.Name << " = " << sig.GetName() << "_" << sigResource.Name << ";\n";
        }

        for (const SIGSampler& sigSampler : sig.GetSamplers())
        {
            ss << "\tresources." << sigSampler.Name << " = " << sig.GetName() << "_" << sigSampler.Name << ";\n";
        }

        ss << "\treturn resources;\n";
        ss << "}\n\n";

        ss << "#endif // __" << sig.GetName() << "_HLSLI__\n";

        // Create file on disk
        std::string outputPath = m_OutputDirectory.string() + "/hlsl/" + sig.GetName() + ".hlsli";
        std::ofstream ofs(outputPath, std::ios::out | std::ios::trunc);

        if (!ofs.is_open())
            throw std::exception(std::format("File {} could not be created", outputPath.c_str()).c_str());

        ofs << ss.str();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SIGParser::GenerateHlslFile(const SIGStructType& sigStruct)
    {
        std::ostringstream ss;

        ss << "#ifndef __" << sigStruct.GetName() << "_HLSLI__\n";
        ss << "#define __" << sigStruct.GetName() << "_HLSLI__\n\n";

        ss << "struct " << sigStruct.GetName() << "\n{\n";

        for (const SIGConstant& member : sigStruct.GetMembers())
            ss << "\t" << Utils::SIGConstantTypeToHlslString(member.Type) << " " << member.Name << ";\n";

        ss << "};\n\n";

        ss << "#endif // __" << sigStruct.GetName() << "_HLSLI__\n";

        // Create file on disk
        std::string outputPath = m_OutputDirectory.string() + "/hlsl/" + sigStruct.GetName() + ".hlsli";
        std::ofstream ofs(outputPath, std::ios::out | std::ios::trunc);

        if (!ofs.is_open())
            throw std::exception(std::format("File {} could not be created", outputPath.c_str()).c_str());

        ofs << ss.str();
    }
}
