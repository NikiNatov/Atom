#pragma once

#include "SIGTypes.h"

#include <string>
#include <vector>
#include <filesystem>

namespace SIGCompiler
{
    class SIGParser
    {
    public:
        SIGParser(const std::filesystem::path& sigFilepath);

        void Parse();
        void GenerateCppFile(const std::filesystem::path& outputPath);
        void GenerateHlslFile(const std::filesystem::path& outputPath);
    private:
        std::filesystem::path m_SIGFilepath;

        std::vector<SIGStructType> m_SIGStructs;
        SIGStructType* m_CurrentSIGStruct = nullptr;

        std::vector<SIGDeclaration> m_SIGDeclarations;
        SIGDeclaration* m_CurrentSIGDeclaration = nullptr;
    };
}