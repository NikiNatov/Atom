#include "SIGParser.h"

#include <iostream>

int main(int argc, char** argv)
{
    std::filesystem::path sigInputDirectory;
    std::filesystem::path sigOutputDirectory;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp("-sigInputDir", argv[i]) == 0)
        {
            sigInputDirectory = argv[i + 1];
        }
        else if (strcmp("-sigOutputDir", argv[i]) == 0)
        {
            sigOutputDirectory = argv[i + 1];
        }
    }

    try
    {
        if (!std::filesystem::exists(sigInputDirectory))
        {
            std::cout << "SIG input directory does not exist!\n";
            return -1;
        }

        std::filesystem::path hlslOutputPath = sigOutputDirectory / "hlsl";
        std::filesystem::create_directories(hlslOutputPath);

        std::filesystem::path headerOutputPath = sigOutputDirectory / "cpp";
        std::filesystem::create_directories(headerOutputPath);

        // Parse all SIGs and generate cpp and hlsl files
        for (std::filesystem::directory_entry entry : std::filesystem::recursive_directory_iterator(sigInputDirectory))
        {
            std::filesystem::path sigFilepath = entry.path();

            if (sigFilepath.extension() != ".sig")
                continue;

            std::cout << "Parsing " << sigFilepath.string() << "\n";

            SIGCompiler::SIGParser parser(entry.path());
            parser.Parse();
            parser.GenerateCppFile(headerOutputPath / (sigFilepath.stem().string() + ".h"));
            parser.GenerateHlslFile(hlslOutputPath / (sigFilepath.stem().string() + ".hlsli"));
        }
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}