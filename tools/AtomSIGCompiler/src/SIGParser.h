#pragma once

#include "SIGTypes.h"

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

namespace SIGCompiler
{
    class SIGParser
    {
    public:
        SIGParser(const std::filesystem::path& inputDirectory, const std::filesystem::path& outputDirectory);

        void Parse();
    private:
        void ParseSIGFile(const std::filesystem::path& sigFilepath);
        void GenerateSIGDataBaseFile();
        void GenerateCppFile(const SIGLayoutDeclaration& sigLayout);
        void GenerateHlslFile(const SIGLayoutDeclaration& sigLayout);
        void GenerateCppFile(const SIGDeclaration& sig);
        void GenerateHlslFile(const SIGDeclaration& sig);
        void GenerateHlslFile(const SIGStructType& sigStruct);
    private:
        std::filesystem::path m_InputDirectory;
        std::filesystem::path m_OutputDirectory;

        std::vector<SIGStructType> m_SIGStructs;
        SIGStructType* m_CurrentSIGStruct = nullptr;

        std::vector<SIGDeclaration> m_SIGDeclarations;
        SIGDeclaration* m_CurrentSIGDeclaration = nullptr;

        std::vector<SIGLayoutDeclaration> m_SIGLayouts;
        SIGLayoutDeclaration* m_CurrentSIGLayout = nullptr;

        std::vector<SIGBindPoint> m_SIGBindPoints;
        std::unordered_map<std::string, uint32_t> m_BindPointFullNameToID;

        std::vector<SIGStaticSampler> m_SIGStaticSamplers;
        std::unordered_map<std::string, uint32_t> m_StaticSamplerFullNameToID;

        std::unordered_set<std::string> m_ExistingNames;

    };
}