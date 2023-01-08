#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    class NewProjectDialog
    {
    public:
        void Open();
        void OnImGuiRender();

    protected:
        bool                  m_IsOpened = false;
        String                m_ProjectName;
        String                m_StartSceneName;
        std::filesystem::path m_ProjectLocation;
    };
}