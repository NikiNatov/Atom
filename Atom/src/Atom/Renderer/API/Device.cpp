#include "atompch.h"
#include "Device.h"

#include "Atom/Platform/DirectX12/DX12Device.h"

namespace Atom
{
    Ref<Device> Device::Create(GPUPreference gpuPreference)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12Device>(gpuPreference);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}
