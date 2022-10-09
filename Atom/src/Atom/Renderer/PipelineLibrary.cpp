#include "atompch.h"
#include "PipelineLibrary.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void PipelineLibrary::Clear()
    {
        m_PipelineMap.clear();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    bool PipelineLibrary::Exists(const String& name) const
    {
        return m_PipelineMap.find(name) != m_PipelineMap.end();
    }
}
