#include "atompch.h"
#include "Shader.h"

#include "Atom/Platform/DirectX12/DX12Shader.h"

namespace Atom
{
    Ref<Shader> Shader::Create(const String& filepath)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12Shader>(filepath);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}
