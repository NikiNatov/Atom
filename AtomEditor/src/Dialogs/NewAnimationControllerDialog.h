#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Asset/AnimationControllerAsset.h"

namespace Atom
{
    class NewAnimationControllerDialog
    {
    public:
        void Open();
        void OnImGuiRender();

    protected:
        bool                          m_IsOpened = false;
        String                        m_ControllerName;
        u32                           m_InitialStateIdx = 0;
        Vector<std::filesystem::path> m_AnimationPaths;
    };
}