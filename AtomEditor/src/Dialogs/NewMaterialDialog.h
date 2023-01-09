#pragma once

#include "Atom/Core/Core.h"

namespace Atom
{
    class NewMaterialDialog
    {
    public:
        void Open();
        void OnImGuiRender();

    protected:
        bool   m_IsOpened = false;
        String m_MaterialName;
    };
}