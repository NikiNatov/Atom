#include "atompch.h"
#include "ShaderLibrary.h"

namespace Atom
{
	// -----------------------------------------------------------------------------------------------------------------------------
	ShaderLibrary::ShaderLibrary()
	{
		ATOM_ENGINE_ASSERT(ms_Instance == nullptr, "Shader library already exists!");
		ms_Instance = this;
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	Ref<GraphicsShader> ShaderLibrary::LoadGraphicsShader(const std::filesystem::path& filepath)
	{
		Ref<GraphicsShader> shader = ShaderCompiler::CompileGraphicsShader(filepath);

		if (shader)
			Add<GraphicsShader>(shader);

		return shader;
	}

	// -----------------------------------------------------------------------------------------------------------------------------
	Ref<ComputeShader> ShaderLibrary::LoadComputeShader(const std::filesystem::path& filepath)
	{
		Ref<ComputeShader> shader = ShaderCompiler::CompileComputeShader(filepath);

		if (shader)
			Add<ComputeShader>(shader);

		return shader;
	}

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
