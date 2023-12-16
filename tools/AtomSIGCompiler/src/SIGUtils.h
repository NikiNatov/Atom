#pragma once

namespace SIGCompiler
{
    namespace Utils
    {
        static SIGConstantType StringToSIGConstantType(const std::string& str)
        {
            if (str == "int")
                return SIGConstantType::Int;
            if (str == "int2")
                return SIGConstantType::Int2;
            if (str == "int3")
                return SIGConstantType::Int3;
            if (str == "int4")
                return SIGConstantType::Int4;
            if (str == "uint")
                return SIGConstantType::Uint;
            if (str == "uint2")
                return SIGConstantType::Uint2;
            if (str == "uint3")
                return SIGConstantType::Uint3;
            if (str == "uint4")
                return SIGConstantType::Uint4;
            if (str == "float")
                return SIGConstantType::Float;
            if (str == "float2")
                return SIGConstantType::Float2;
            if (str == "float3")
                return SIGConstantType::Float3;
            if (str == "float4")
                return SIGConstantType::Float4;
            if (str == "bool")
                return SIGConstantType::Bool;
            if (str == "matrix")
                return SIGConstantType::Matrix;

            return SIGConstantType::None;
        }

        static SIGResourceType StringToSIGResourceType(const std::string& str)
        {
            if (str == "Texture2D")
                return SIGResourceType::Texture2D;
            if (str == "Texture2DArray")
                return SIGResourceType::Texture2DArray;
            if (str == "Texture3D")
                return SIGResourceType::Texture3D;
            if (str == "TextureCube")
                return SIGResourceType::TextureCube;
            if (str == "StructuredBuffer")
                return SIGResourceType::StructuredBuffer;
            if (str == "RWTexture2D")
                return SIGResourceType::RWTexture2D;
            if (str == "RWTexture2DArray")
                return SIGResourceType::RWTexture2DArray;
            if (str == "RWTexture3D")
                return SIGResourceType::RWTexture3D;
            if (str == "RWStructuredBuffer")
                return SIGResourceType::RWStructuredBuffer;

            return SIGResourceType::None;
        }

        static bool IsSIGResourceReadOnly(SIGResourceType type)
        {
            switch (type)
            {
            case SIGResourceType::Texture2D:
            case SIGResourceType::Texture2DArray:
            case SIGResourceType::Texture3D:
            case SIGResourceType::TextureCube:
            case SIGResourceType::StructuredBuffer:
                return true;
            case SIGResourceType::RWTexture2D:
            case SIGResourceType::RWTexture2DArray:
            case SIGResourceType::RWTexture3D:
            case SIGResourceType::RWStructuredBuffer:
                return false;
            }

            return false;
        }

        static bool IsSIGResourceTexture(SIGResourceType type)
        {
            switch (type)
            {
            case SIGResourceType::Texture2D:
            case SIGResourceType::Texture2DArray:
            case SIGResourceType::Texture3D:
            case SIGResourceType::TextureCube:
            case SIGResourceType::RWTexture2D:
            case SIGResourceType::RWTexture2DArray:
            case SIGResourceType::RWTexture3D:
                return true;
            }

            return false;
        }

        static uint32_t GetSIGConstantSize(SIGConstantType type)
        {
            switch (type)
            {
            case SIGConstantType::Int:    return 4;
            case SIGConstantType::Int2:   return 4 * 2;
            case SIGConstantType::Int3:   return 4 * 3;
            case SIGConstantType::Int4:   return 4 * 4;
            case SIGConstantType::Uint:   return 4;
            case SIGConstantType::Uint2:  return 4 * 2;
            case SIGConstantType::Uint3:  return 4 * 3;
            case SIGConstantType::Uint4:  return 4 * 4;
            case SIGConstantType::Float:  return 4;
            case SIGConstantType::Float2: return 4 * 2;
            case SIGConstantType::Float3: return 4 * 3;
            case SIGConstantType::Float4: return 4 * 4;
            case SIGConstantType::Bool:   return 4;
            case SIGConstantType::Matrix: return 4 * 4 * 4;
            }

            return 0;
        }

        static std::string SIGConstantTypeToHlslString(SIGConstantType type)
        {
            switch (type)
            {
            case SIGConstantType::Int:    return "int";
            case SIGConstantType::Int2:   return "int2";
            case SIGConstantType::Int3:   return "int3";
            case SIGConstantType::Int4:   return "int4";
            case SIGConstantType::Uint:   return "uint";
            case SIGConstantType::Uint2:  return "uint2";
            case SIGConstantType::Uint3:  return "uint3";
            case SIGConstantType::Uint4:  return "uint4";
            case SIGConstantType::Float:  return "float";
            case SIGConstantType::Float2: return "float2";
            case SIGConstantType::Float3: return "float3";
            case SIGConstantType::Float4: return "float4";
            case SIGConstantType::Bool:   return "bool";
            case SIGConstantType::Matrix: return "matrix";
            }

            return "";
        }

        static std::string SIGConstantTypeToCppString(SIGConstantType type)
        {
            switch (type)
            {
            case SIGConstantType::Int:    return "s32";
            case SIGConstantType::Int2:   return "glm::ivec2";
            case SIGConstantType::Int3:   return "glm::ivec3";
            case SIGConstantType::Int4:   return "glm::ivec4";
            case SIGConstantType::Uint:   return "u32";
            case SIGConstantType::Uint2:  return "glm::uvec2";
            case SIGConstantType::Uint3:  return "glm::uvec3";
            case SIGConstantType::Uint4:  return "glm::uvec4";
            case SIGConstantType::Float:  return "f32";
            case SIGConstantType::Float2: return "glm::vec2";
            case SIGConstantType::Float3: return "glm::vec3";
            case SIGConstantType::Float4: return "glm::vec4";
            case SIGConstantType::Bool:   return "bool";
            case SIGConstantType::Matrix: return "glm::mat4";
            }

            return "";
        }

        static std::string SIGResourceTypeToHlslString(SIGResourceType type)
        {
            switch (type)
            {
            case SIGResourceType::Texture2D:          return "Texture2D";
            case SIGResourceType::Texture2DArray:     return "Texture2DArray";
            case SIGResourceType::Texture3D:          return "Texture3D";
            case SIGResourceType::TextureCube:        return "TextureCube";
            case SIGResourceType::StructuredBuffer:   return "StructuredBuffer";
            case SIGResourceType::RWTexture2D:        return "RWTexture2D";
            case SIGResourceType::RWTexture2DArray:   return "RWTexture2DArray";
            case SIGResourceType::RWTexture3D:        return "RWTexture3D";
            case SIGResourceType::RWStructuredBuffer: return "RWStructuredBuffer";
            }

            return "";
        }

        static std::string SIGResourceTypeToCppString(SIGResourceType type)
        {
            switch (type)
            {
            case SIGResourceType::Texture2D:          return "Texture";
            case SIGResourceType::Texture2DArray:     return "Texture";
            case SIGResourceType::Texture3D:          return "Texture";
            case SIGResourceType::TextureCube:        return "Texture";
            case SIGResourceType::StructuredBuffer:   return "StructuredBuffer";
            case SIGResourceType::RWTexture2D:        return "Texture";
            case SIGResourceType::RWTexture2DArray:   return "Texture";
            case SIGResourceType::RWTexture3D:        return "Texture";
            case SIGResourceType::RWStructuredBuffer: return "StructuredBuffer";
            }

            return "";
        }

        static SamplerFilter StringToSamplerFilter(const std::string& str)
        {
            if (str == "SamplerFilter::Linear")
                return SamplerFilter::Linear;
            if (str == "SamplerFilter::Nearest")
                return SamplerFilter::Nearest;
            if (str == "SamplerFilter::Anisotropic")
                return SamplerFilter::Anisotropic;

            return SamplerFilter::None;
        }

        static SamplerWrap StringToSamplerWrap(const std::string& str)
        {
            if (str == "SamplerWrap::Clamp")
                return SamplerWrap::Clamp;
            if (str == "SamplerWrap::Repeat")
                return SamplerWrap::Repeat;

            return SamplerWrap::None;
        }

        static std::string SamplerFilterToHlslString(SamplerFilter filter)
        {
            switch (filter)
            {
            case SamplerFilter::Linear:      return "FILTER_MIN_MAG_MIP_LINEAR";
            case SamplerFilter::Nearest:     return "FILTER_MIN_MAG_MIP_POINT";
            case SamplerFilter::Anisotropic: return "FILTER_ANISOTROPIC";
            }

            return "";
        }

        static std::string SamplerFilterToCppString(SamplerFilter filter)
        {
            switch (filter)
            {
            case SamplerFilter::Linear:      return "TextureFilter::Linear";
            case SamplerFilter::Nearest:     return "TextureFilter::Nearest";
            case SamplerFilter::Anisotropic: return "TextureFilter::Anisotropic";
            }

            return "";
        }

        static std::string SamplerWrapToHlslString(SamplerWrap wrap)
        {
            switch (wrap)
            {
            case SamplerWrap::Clamp:  return "TEXTURE_ADDRESS_CLAMP";
            case SamplerWrap::Repeat: return "TEXTURE_ADDRESS_WRAP";
            }

            return "";
        }

        static std::string SamplerWrapToCppString(SamplerWrap wrap)
        {
            switch (wrap)
            {
            case SamplerWrap::Clamp:  return "TextureWrap::Clamp";
            case SamplerWrap::Repeat: return "TextureWrap::Repeat";
            }

            return "";
        }
    }
}