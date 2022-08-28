#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Core/DirectX12/DirectX12.h"
#include "Shader.h"

namespace Atom
{
    class Framebuffer;

    class PipelineLayout
    {
    public:
        struct Element
        {
            String         SemanticName;
            ShaderDataType Type;
            u32            Offset = 0;

            Element(const String& semantic, ShaderDataType type)
                : SemanticName(semantic), Type(type)
            {}

            u32 GetElementSize()
            {
                switch (Type)
                {
                    case ShaderDataType::Unorm4: return 1 * 4;
                    case ShaderDataType::Int:    return 4;
                    case ShaderDataType::Int2:   return 4 * 2;
                    case ShaderDataType::Int3:   return 4 * 3;
                    case ShaderDataType::Int4:   return 4 * 4;
                    case ShaderDataType::Uint:   return 4;
                    case ShaderDataType::Uint2:  return 4 * 2;
                    case ShaderDataType::Uint3:  return 4 * 3;
                    case ShaderDataType::Uint4:  return 4 * 4;
                    case ShaderDataType::Float:  return 4;
                    case ShaderDataType::Float2: return 4 * 2;
                    case ShaderDataType::Float3: return 4 * 3;
                    case ShaderDataType::Float4: return 4 * 4;
                    case ShaderDataType::Bool:   return 4;
                    case ShaderDataType::Mat2:   return 4 * 2 * 2;
                    case ShaderDataType::Mat3:   return 4 * 3 * 3;
                    case ShaderDataType::Mat4:   return 4 * 4 * 4;
                }

                ATOM_ASSERT(false, "Unknown shader data type!");
                return 0;
            }
        };
    public:
        PipelineLayout() = default;
        PipelineLayout(const std::initializer_list<Element>& elementList)
            : m_Elements(elementList), m_VertexStride(0)
        {
            u32 offset = 0;

            // Calculate layout vertex stride and offsets for each element
            for (auto& element : m_Elements)
            {
                element.Offset = offset;
                offset += element.GetElementSize();
                m_VertexStride += element.GetElementSize();
            }
        }

        inline const Vector<Element>& GetElements() const { return m_Elements; }
        inline u32 GetStride() const { return m_VertexStride; }

        inline Vector<Element>::iterator begin() { return m_Elements.begin(); }
        inline Vector<Element>::const_iterator begin() const { return m_Elements.begin(); }
        inline Vector<Element>::iterator end() { return m_Elements.end(); }
        inline Vector<Element>::const_iterator end() const { return m_Elements.end(); }
    private:
        Vector<Element> m_Elements;
        u32             m_VertexStride;
    };

    enum class Topology
    {
        Triangles,
        Points,
        Lines
    };

    struct GraphicsPipelineDescription
    {
        PipelineLayout   Layout;
        Ref<Shader>      Shader;
        Ref<Framebuffer> Framebuffer;
        Topology         Topology;
        bool             EnableDepthTest;
        bool             EnableBlend;
        bool             Wireframe;
        bool             BackfaceCulling;
    };

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(const GraphicsPipelineDescription& description, const char* debugName = "Unnamed Graphics Pipeline");
        ~GraphicsPipeline();

        const PipelineLayout& GetLayout() const;
        const Shader* GetShader() const;
        const Framebuffer* GetFramebuffer() const;
        Topology GetTopology() const;
        bool IsDepthTestingEnabled() const;
        bool IsBlendingEnabled() const;
        bool IsWireframe() const;
        bool IsBackfaceCullingEnabled() const;
        inline ComPtr<ID3D12PipelineState> GetD3DPipeline() const { return m_D3DPipeline; }
        inline const D3D12_GRAPHICS_PIPELINE_STATE_DESC& GetD3DDescription() const { return m_D3DDescription; }
    private:
        GraphicsPipelineDescription        m_Description;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC m_D3DDescription{};
        ComPtr<ID3D12PipelineState>        m_D3DPipeline;
    };
}