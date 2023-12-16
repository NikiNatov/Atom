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

        SIGCompiler::SIGParser parser(sigInputDirectory, sigOutputDirectory);
        parser.Parse();
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}