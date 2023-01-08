#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    class AssetPanel
    {
    public:
        AssetPanel();

        void OnImGuiRender();
    private:
        std::filesystem::path m_AssetsDirectory;
        std::filesystem::path m_CurrentDirectory;
    };
}