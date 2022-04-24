#include "atompch.h"
#include "Buffer.h"

#include "Atom/Platform/DirectX12/DX12Buffer.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<VertexBuffer> VertexBuffer::Create(const VertexBufferDescription& description, const char* debugName)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12VertexBuffer>(description, debugName);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }


    // -----------------------------------------------------------------------------------------------------------------------------
    Ref<IndexBuffer> IndexBuffer::Create(const IndexBufferDescription& description, const char* debugName)
    {
        switch (Renderer::GetAPI())
        {

#if defined(ATOM_PLATFORM_WINDOWS)
            case RenderAPI::DirectX12: return CreateRef<DX12IndexBuffer>(description, debugName);
#endif // ATOM_PLATFORM_WINDOWS

        }

        ATOM_ASSERT(false, "Unknown API!");
        return nullptr;
    }
}