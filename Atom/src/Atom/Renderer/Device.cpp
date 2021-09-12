#include "atompch.h"
#include "Device.h"

#include "Platform/DirectX12/DX12Device.h"

namespace Atom
{
    Scope<Device> Device::CreateDevice(const Adapter* adapter)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DirectX12: return CreateScope<DX12Device>(adapter);
        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}
