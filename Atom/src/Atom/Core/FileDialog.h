#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    class FileDialog
    {
    public:
        static std::filesystem::path OpenFile(const char* filters);
        static std::filesystem::path SaveFile(const char* filters);
    };
}