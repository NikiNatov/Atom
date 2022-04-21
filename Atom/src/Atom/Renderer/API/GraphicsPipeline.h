#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/API/Shader.h"
#include "Atom/Renderer/API/Framebuffer.h"

namespace Atom
{
    class DX12GraphicsPipeline;

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
        virtual ~GraphicsPipeline() = default;

        virtual const PipelineLayout& GetLayout() const = 0;
        virtual const Ref<Shader>& GetShader() const = 0;
        virtual const Ref<Framebuffer>& GetFramebuffer() const = 0;
        virtual Topology GetTopology() const = 0;
        virtual bool IsDepthTestingEnabled() const = 0;
        virtual bool IsBlendingEnabled() const = 0;
        virtual bool IsWireframe() const = 0;
        virtual bool IsBackfaceCullingEnabled() const = 0;

        IMPL_API_CAST(GraphicsPipeline)

        static Ref<GraphicsPipeline> Create(const GraphicsPipelineDescription& description, const char* debugName = "Unnamed Graphics Pipeline");
    };
}