#include "atompch.h"
#include "ShaderLibrary.h"

#include "Atom/Renderer/Shader.h"

namespace Atom
{
	// -----------------------------------------------------------------------------------------------------------------------------
	void ShaderLibrary::Clear()
	{
		m_ShaderMap.clear();
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	bool ShaderLibrary::Exists(const String& name) const
	{
		return m_ShaderMap.find(name) != m_ShaderMap.end();
	}
}
