#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/DescriptorHeap.h"
#include "Atom/Renderer/ShaderLayout.h"
#include "Atom/Renderer/Buffer.h"

namespace Atom
{
    class ShaderInputGroup
    {
    public:
        virtual ~ShaderInputGroup() = default;

        virtual void Compile() = 0;

        virtual ShaderBindPoint GetBindPoint() const = 0;
        virtual const byte* GetRootConstantsData() const = 0;
        virtual const Ref<ConstantBuffer>& GetConstantBuffer() const = 0;
        virtual const DescriptorAllocation& GetResourceTable() const = 0;
        virtual const DescriptorAllocation& GetSamplerTable() const = 0;
    };
}