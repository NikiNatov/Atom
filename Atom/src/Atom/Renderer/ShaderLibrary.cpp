#include "atompch.h"
#include "ShaderLibrary.h"

#include "Atom/Renderer/Shader.h"

namespace Atom
{
	// -----------------------------------------------------------------------------------------------------------------------------
	void ShaderLibrary::Add(const String& name, const Ref<Shader>& shader)
	{
		bool exists = Exists(name);
		ATOM_ENGINE_ASSERT(!exists, "Shader with that name already exists!");

		if (!exists)
			m_ShaderMap[name] = shader;
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		auto& name = shader->GetName();
		Add(name, shader);
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	Ref<Shader> ShaderLibrary::Load(const String& name, const String& filepath)
	{
		Ref<Shader> shader = CreateRef<Shader>(filepath);
		Add(name, shader);
		return shader;
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	Ref<Shader> ShaderLibrary::Load(const String& filepath)
	{
		Ref<Shader> shader = CreateRef<Shader>(filepath);
		Add(shader);
		return shader;
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	Ref<Shader> ShaderLibrary::Get(const String& name) const
	{
		ATOM_ENGINE_ASSERT(Exists(name), "Shader does not exist!");
		return m_ShaderMap.at(name);
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	bool ShaderLibrary::Exists(const String& name) const
	{
		return m_ShaderMap.find(name) != m_ShaderMap.end();
	}
}
