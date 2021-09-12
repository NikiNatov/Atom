#include "atompch.h"
#include "Adapter.h"

#include "Platform/DirectX12/DX12Adapter.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Scope<Adapter> Adapter::CreateAdapter(AdapterPreference preference)
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::DirectX12: return CreateScope<DX12Adapter>(preference);
        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}
