#include "atompch.h"
#include "ShaderInputGroup.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void ShaderInputGroup::Compile()
    {
        m_SIGStorage.CopyDescriptors();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    ShaderInputGroup::ShaderInputGroup(const ShaderInputGroupStorage& sigStorage)
        : m_SIGStorage(sigStorage)
    {
    }
}
