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
    SIGParser::SIGParser(const std::filesystem::path& sigFilepath)
        : m_SIGFilepath(sigFilepath)
    {
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SIGParser::Parse()
    {
        if (!std::filesystem::exists(m_SIGFilepath))
            throw std::exception(std::format("File {} does not exist", m_SIGFilepath.string().c_str()).c_str());

        std::ifstream ifs(m_SIGFilepath, std::ios::in);

        if (!ifs.is_open())
            throw std::exception(std::format("File {} could not be opened", m_SIGFilepath.string().c_str()).c_str());

        // Read file
        std::stringstream fileStringStream;
        fileStringStream << ifs.rdbuf();

        // Parse
        SIGTokenizer tokenizer(fileStringStream.str());
        while (SIGToken token = (++tokenizer).GetCurrentToken())
        {
            if (token.Type == SIGTokenType::Unknown)
                throw std::exception("Syntax error: Unknown token found");

            if (token.Type == SIGTokenType::ShaderInputGroup)
            {
                if (m_CurrentSIGDeclaration || m_CurrentSIGStruct)
                    throw std::exception("Syntax error: Missing \"}\"");

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::LeftAngleBracket, "Syntax error: Expected a \"<\"");
                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::BindPoint, "Syntax error: Expected a bind point specifier");

                SIGBindPoint sigBindPoint = Utils::StringToSIGBindPoint(token.Value);

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::RightAngleBracket, "Syntax error: Missing \">\"");
                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected shader input group identifier");

                std::string sigName = token.Value;

                m_SIGDeclarations.emplace_back(sigName, sigBindPoint);
                m_CurrentSIGDeclaration = &m_SIGDeclarations.back();

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::LeftCurlyBracket, "Syntax error: Expected a \"{\"");
            }
            else if (token.Type == SIGTokenType::Struct)
            {
                if (m_CurrentSIGDeclaration || m_CurrentSIGStruct)
                    throw std::exception("Syntax error: Missing \"}\"");

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::Identifier, "Syntax error: Expected struct identifier");

                std::string sigStructName = token.Value;

                m_SIGStructs.emplace_back(sigStructName);
                m_CurrentSIGStruct = &m_SIGStructs.back();

                token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::LeftCurlyBracket, "Syntax error: Expected a \"{\"");
            }
            else if (token.Type == SIGTokenType::BaseType || token.Type == SIGTokenType::Matrix)
            {
                if(!m_CurrentSIGDeclaration && !m_CurrentSIGStruct)
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
                std::variant<SIGConstantType, SIGStructType> sigResourceReturnType = SIGConstantType::Float4;

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

                    // Find if the struct idendifier exists
                    SIGStructType* structPtr = nullptr;
                    for (auto& sigStruct : m_SIGStructs)
                    {
                        if (sigStruct.GetName() == token.Value)
                        {
                            structPtr = &sigStruct;
                            break;
                        }
                    }

                    if(!structPtr)
                        throw std::exception("Syntax error: Unknown type used as return type");

                    sigResourceReturnType = *structPtr;
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

                    if(arraySize == 0)
                        throw std::exception("Syntax error: Array size cannot be 0");

                    token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::RightSquareBracket, "Syntax error: Expected a \"]\"");
                    token = (++tokenizer).GetAndValidateCurrentToken(SIGTokenType::SemiColon, "Syntax error: Expected a \";\"");

                    if (!m_CurrentSIGDeclaration->AddResource(sigResourceName, sigResourceType, sigResourceReturnType, arraySize))
                        throw std::exception(std::format("Syntax error: Resource name redefinition \"{}\"", sigResourceName.c_str()).c_str());
                }
                else if (token.Type == SIGTokenType::SemiColon)
                {
                    if (!m_CurrentSIGDeclaration->AddResource(sigResourceName, sigResourceType, sigResourceReturnType))
                        throw std::exception(std::format("Syntax error: Resource name redefinition \"{}\"", sigResourceName.c_str()).c_str());
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

                    if (!m_CurrentSIGDeclaration->AddSampler(sigSamplerName, arraySize))
                        throw std::exception(std::format("Syntax error: Sampler name redefinition \"{}\"", sigSamplerName.c_str()).c_str());
                }
                else if (token.Type == SIGTokenType::SemiColon)
                {
                    if (!m_CurrentSIGDeclaration->AddSampler(sigSamplerName))
                        throw std::exception(std::format("Syntax error: Sampler name redefinition \"{}\"", sigSamplerName.c_str()).c_str());
                }
                else
                    throw std::exception("Syntax error: Expected a \";\"");
            }
            else if (token.Type == SIGTokenType::RightCurlyBracket)
            {
                if(!m_CurrentSIGDeclaration && !m_CurrentSIGStruct)
                    throw std::exception("Syntax error: Unexpected \"}\"");

                m_CurrentSIGDeclaration = nullptr;
                m_CurrentSIGStruct = nullptr;
            }
            else if(token.Type == SIGTokenType::SemiColon)
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
    void SIGParser::GenerateCppFile(const std::filesystem::path& outputPath)
    {
        std::ostringstream ss;

        ss << "#pragma once\n\n";
        ss << "#include \"Atom/Core/Core.h\"\n";
        ss << "#include \"Atom/Renderer/ShaderInputGroup.h\"\n";
        ss << "#include \"Atom/Renderer/Texture.h\"\n";
        ss << "#include \"Atom/Renderer/Buffer.h\"\n";
        ss << "#include \"Atom/Renderer/Device.h\"\n";
        ss << "#include \"Atom/Renderer/DescriptorHeap.h\"\n\n";
        ss << "#include <glm/glm.hpp>\n\n";

        ss << "namespace Atom { namespace SIG {\n";

        for (const SIGDeclaration& sig : m_SIGDeclarations)
        {
            SIGBindPoint bindPoint = sig.GetBindPoint();
            bool hasConstantBuffer = bindPoint == SIGBindPoint::Frame || bindPoint == SIGBindPoint::Pass;

            ss << "\tclass " << sig.GetName() << " : public ShaderInputGroup\n";
            ss << "\t{\n";
            ss << "\tpublic:\n";

            // Write constructor
            ss << "\t\t" << sig.GetName() << "()\n";
            ss << "\t\t{\n";

            if (!sig.GetConstants().empty())
            {
                ss << "\t\t\tmemset(m_ConstantsData, 0, ConstantsDataSize);\n\n";

                if (hasConstantBuffer)
                {
                    ss << "\t\t\tBufferDescription desc;\n";
                    ss << "\t\t\tdesc.ElementCount = 1;\n";
                    ss << "\t\t\tdesc.ElementSize = 256 * ((ConstantsDataSize + 255) / 256);\n";
                    ss << "\t\t\tdesc.IsDynamic = true;\n\n";
                    ss << "\t\t\tm_ConstantBuffer = CreateRef<ConstantBuffer>(desc, \"" << sig.GetName() << "_ConstantBuffer\");\n\n";
                }
            }

            ss << "\t\t}\n\n";

            // Write destructor
            ss << "\t\t~" << sig.GetName() << "()\n";
            ss << "\t\t{\n";

            if (!sig.GetResources().empty())
            {
                if (bindPoint == SIGBindPoint::Material)
                {
                    ss << "\t\t\tif (m_ResourceTable.IsValid())\n";
                    ss << "\t\t\t\tDevice::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource)->Release(std::move(m_ResourceTable), true);\n";
                }
            }

            if (!sig.GetSamplers().empty())
            {
                if (bindPoint == SIGBindPoint::Material)
                {
                    ss << "\t\t\tif (m_SamplerTable.IsValid())\n";
                    ss << "\t\t\t\tDevice::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler)->Release(std::move(m_SamplerTable), true);\n";
                }
            }

            ss << "\t\t}\n\n";

            // Write constants setters
            uint32_t currentByteOffset = 0;
            for (const SIGConstant& sigConst : sig.GetConstants())
            {
                ss << "\t\tvoid Set" << sigConst.Name << "(const " << Utils::SIGConstantTypeToCppString(sigConst.Type) << "& value)\n";
                ss << "\t\t{\n";
                ss << "\t\t\tmemcpy(m_ConstantsData + " << currentByteOffset << ", &value, " << sigConst.SizeInBytes << ");\n";

                if (hasConstantBuffer)
                {
                    ss << "\t\t\tm_ConstantsDirty = true;\n";
                }

                ss << "\t\t}\n\n";

                currentByteOffset += sigConst.SizeInBytes;
            }

            // Write resources setters
            uint32_t numROResources = 0;
            uint32_t numRWResources = 0;
            uint32_t numSamplers = 0;
            for (const SIGResource& sigResource : sig.GetResources())
            {
                ss << "\t\tvoid Set" << sigResource.Name;

                if (sigResource.ArraySize > 1)
                    ss << "AtIndex(u32 index, ";
                else
                    ss << "(";

                ss << "const " << Utils::SIGResourceTypeToCppString(sigResource.Type) << "& value)\n";
                ss << "\t\t{\n";

                if (sigResource.ArraySize > 1)
                    ss << "\t\t\tATOM_ENGINE_ASSERT(index < " << sigResource.ArraySize << ");\n";

                if (Utils::IsSIGResourceReadOnly(sigResource.Type))
                {
                    ss << "\t\t\tm_ResourceDescriptors[" << sigResource.ShaderRegister;

                    if (sigResource.ArraySize > 1)
                        ss << " + index";

                    ss << "] = value->GetSRV()" << (Utils::IsSIGResourceTexture(sigResource.Type) ? "->GetDescriptor();\n" : ";\n");

                    numROResources += sigResource.ArraySize;
                }
                else
                {
                    ss << "\t\t\tm_ResourceDescriptors[NumROResources + " << sigResource.ShaderRegister;

                    if (sigResource.ArraySize > 1)
                        ss << " + index";

                    ss << "] = value->GetUAV()" << (Utils::IsSIGResourceTexture(sigResource.Type) ? "->GetDescriptor();\n" : ";\n");

                    numRWResources += sigResource.ArraySize;
                }

                ss << "\t\t\tm_ResourcesDirty = true;\n";
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

                ss<< "const Ref<TextureSampler>& value)\n";

                ss << "\t\t{\n";

                if (sigSampler.ArraySize > 1)
                    ss << "\t\t\tATOM_ENGINE_ASSERT(index < " << sigSampler.ArraySize << ");\n";

                ss << "\t\t\tm_SamplerDescriptors[" << sigSampler.ShaderRegister;

                if (sigSampler.ArraySize > 1)
                    ss << " + index";

                ss << "] = value->GetDescriptor();\n";
                ss << "\t\t\tm_SamplersDirty = true;\n";
                ss << "\t\t}\n\n";

                numSamplers += sigSampler.ArraySize;
            }

            // Write compile method
            ss << "\t\tvirtual void Compile() override\n";
            ss << "\t\t{\n";

            if (!sig.GetConstants().empty())
            {
                if (hasConstantBuffer)
                {
                    ss << "\t\t\tif (m_ConstantsDirty)\n";
                    ss << "\t\t\t{\n";
                    ss << "\t\t\t\tvoid* data = m_ConstantBuffer->Map(0, 0);\n";
                    ss << "\t\t\t\tmemcpy(data, m_ConstantsData, ConstantsDataSize);\n";
                    ss << "\t\t\t\tm_ConstantBuffer->Unmap();\n";
                    ss << "\t\t\t}\n";
                }
            }

            if (!sig.GetResources().empty())
            {
                ss << "\t\t\tif (m_ResourcesDirty)\n";
                ss << "\t\t\t{\n";
                ss << "\t\t\t\tif (!m_ResourceTable.IsValid())\n";
                ss << "\t\t\t\t\tm_ResourceTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource)->Allocate" << (bindPoint == SIGBindPoint::Material ? "Persistent" : "Transient") << "(NumROResources + NumRWResources);\n\n";
                ss << "\t\t\t\tDevice::Get().CopyDescriptors(m_ResourceTable, NumROResources + NumRWResources, m_ResourceDescriptors, DescriptorHeapType::ShaderResource);\n";
                ss << "\t\t\t}\n";
            }

            if (!sig.GetSamplers().empty())
            {
                ss << "\t\t\tif (m_SamplersDirty)\n";
                ss << "\t\t\t{\n";
                ss << "\t\t\t\tif (!m_SamplerTable.IsValid())\n";
                ss << "\t\t\t\t\tm_SamplerTable = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::Sampler)->Allocate" << (bindPoint == SIGBindPoint::Material ? "Persistent" : "Transient") << "(NumSamplers);\n\n";
                ss << "\t\t\t\tDevice::Get().CopyDescriptors(m_SamplerTable, NumSamplers, m_SamplerDescriptors, DescriptorHeapType::Sampler);\n";
                ss << "\t\t\t}\n";
            }

            ss << "\t\t}\n\n";

            // Write interface getters
            ss << "\t\tvirtual ShaderBindPoint GetBindPoint() const override\n";
            ss << "\t\t{\n";
            ss << "\t\t\treturn " << Utils::SIGBindPointToCppString(bindPoint) << ";\n";
            ss << "\t\t}\n\n";

            ss << "\t\tvirtual const byte* GetRootConstantsData() const override\n";
            ss << "\t\t{\n";
            ss << (sig.GetConstants().empty() || hasConstantBuffer ? "\t\t\treturn nullptr;\n" : "\t\t\treturn m_ConstantsData;\n");
            ss << "\t\t}\n\n";

            ss << "\t\tvirtual const Ref<ConstantBuffer>& GetConstantBuffer() const override\n";
            ss << "\t\t{\n";
            ss << (sig.GetConstants().empty() || !hasConstantBuffer ? "\t\t\treturn nullptr;\n" : "\t\t\treturn m_ConstantBuffer;\n");
            ss << "\t\t}\n\n";

            ss << "\t\tvirtual const DescriptorAllocation& GetResourceTable() const override\n";
            ss << "\t\t{\n";
            ss << (sig.GetResources().empty() ? "\t\t\treturn DescriptorAllocation();\n" : "\t\t\treturn m_ResourceTable;\n");
            ss << "\t\t}\n\n";

            ss << "\t\tvirtual const DescriptorAllocation& GetSamplerTable() const override\n";
            ss << "\t\t{\n";
            ss << (sig.GetSamplers().empty() ? "\t\t\treturn DescriptorAllocation();\n" : "\t\t\treturn m_SamplerTable;\n");
            ss << "\t\t}\n\n";

            // Write data members
            ss << "\tprivate:\n";
            ss << "\t\tstatic const u32 ConstantsDataSize = " << currentByteOffset << ";\n";
            ss << "\t\tstatic const u32 NumROResources = " << numROResources << ";\n";
            ss << "\t\tstatic const u32 NumRWResources = " << numRWResources << ";\n";
            ss << "\t\tstatic const u32 NumSamplers = " << numSamplers << ";\n\n";

            if(!sig.GetConstants().empty())
            {
                ss << "\t\tbyte m_ConstantsData[ConstantsDataSize];\n";

                if (hasConstantBuffer)
                {
                    ss << "\t\tRef<ConstantBuffer> m_ConstantBuffer;\n";
                    ss << "\t\tbool m_ConstantsDirty = false;\n";
                }
            }

            if (!sig.GetResources().empty())
            {
                ss << "\t\tDescriptorAllocation m_ResourceTable;\n";
                ss << "\t\tD3D12_CPU_DESCRIPTOR_HANDLE m_ResourceDescriptors[NumROResources + NumRWResources];\n";
                ss << "\t\tbool m_ResourcesDirty = false;\n";
            }

            if (!sig.GetSamplers().empty())
            {
                ss << "\t\tDescriptorAllocation m_SamplerTable;\n";
                ss << "\t\tD3D12_CPU_DESCRIPTOR_HANDLE m_SamplerDescriptors[NumSamplers];\n";
                ss << "\t\tbool m_SamplersDirty = false;\n";
            }

            ss << "\t};\n\n";
        }

        ss << "}}\n";

        // Create file on disk
        std::ofstream ofs(outputPath, std::ios::out | std::ios::trunc);

        if (!ofs.is_open())
            throw std::exception(std::format("File {} could not be created", outputPath.string().c_str()).c_str());

        ofs << ss.str();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void SIGParser::GenerateHlslFile(const std::filesystem::path& outputPath)
    {
        std::ostringstream ss;

        ss << "#ifndef __" << outputPath.stem().string() << "_HLSLI__\n";
        ss << "#define __" << outputPath.stem().string() << "_HLSLI__\n\n";

        for (const SIGStructType& sigStruct : m_SIGStructs)
        {
            ss << "struct " << sigStruct.GetName() << "\n{\n";

            for (const SIGConstant& member : sigStruct.GetMembers())
                ss << "\t" << Utils::SIGConstantTypeToHlslString(member.Type) << " " << member.Name << ";\n";

            ss << "};\n\n";
        }

        for (const SIGDeclaration& sig : m_SIGDeclarations)
        {
            // Write params struct
            ss << "struct " << sig.GetName() << "\n{\n";

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
                else if (const SIGStructType* structTypePtr = std::get_if<SIGStructType>(&sigResource.ReturnType))
                {
                    ss << structTypePtr->GetName();
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

            // Write constant buffer declaration
            if (!sig.GetConstants().empty())
            {
                ss << "// -------------------------------------------------- Constants --------------------------------------------------- //\n";
                ss << "cbuffer " << sig.GetName() << "_Constants : register(b0, " << Utils::SIGBindPointToHlslString(sig.GetBindPoint()) << ")\n{\n";

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
                    else if (const SIGStructType* structTypePtr = std::get_if<SIGStructType>(&sigResource.ReturnType))
                    {
                        ss << structTypePtr->GetName();
                    }

                    ss << "> " << sig.GetName() << "_" << sigResource.Name;

                    if (sigResource.ArraySize > 1)
                        ss << "[" << sigResource.ArraySize << "]";

                    ss << " : register(" << (Utils::IsSIGResourceReadOnly(sigResource.Type) ? "t" : "u") << sigResource.ShaderRegister << ", " << Utils::SIGBindPointToHlslString(sig.GetBindPoint()) << ");\n";
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
                        ss << "[" << sigSampler.ArraySize << "]";

                    ss << " : register(s" << sigSampler.ShaderRegister << ", " << Utils::SIGBindPointToHlslString(sig.GetBindPoint()) << ");\n";
                }

                ss << "\n";
            }

            // Write params struct creation function
            ss << sig.GetName() << " Create" << sig.GetName() << "()\n{\n";
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

            ss << "\treturn resources;\n}\n\n";
        }

        ss << "#endif // __" << outputPath.stem().string() << "_HLSLI__\n";

        // Create file on disk
        std::ofstream ofs(outputPath, std::ios::out | std::ios::trunc);

        if (!ofs.is_open())
            throw std::exception(std::format("File {} could not be created", outputPath.string().c_str()).c_str());

        ofs << ss.str();
    }
}
