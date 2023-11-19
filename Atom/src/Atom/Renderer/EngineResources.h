#pragma once

#include "Atom/Core/Core.h"
#include "Atom/Renderer/Texture.h"
#include "Atom/Renderer/Buffer.h"
#include "Atom/Renderer/TextureSampler.h"

#include "Atom/Asset/MaterialAsset.h"

namespace Atom
{
    class EngineResources
    {
    public:
        // Textures
        inline static Ref<Texture> BlackTexture;
        inline static Ref<Texture> BlackTextureCube;
        inline static Ref<Texture> ErrorTexture;
        inline static Ref<Texture> BRDFTexture;

        // Materials
        inline static Ref<Material> DefaultMaterial;
        inline static Ref<Material> DefaultMaterialAnimated;
        inline static Ref<Material> ErrorMaterial;
        inline static Ref<Material> ErrorMaterialAnimated;

        // Samplers
        inline static Ref<TextureSampler> LinearClampSampler;
        inline static Ref<TextureSampler> LinearRepeatSampler;
        inline static Ref<TextureSampler> NearestClampSampler;
        inline static Ref<TextureSampler> NearestRepeatSampler;
        inline static Ref<TextureSampler> AnisotropicClampSampler;
        inline static Ref<TextureSampler> AnisotropicRepeatSampler;

        // Buffers
        inline static Ref<VertexBuffer> QuadVertexBuffer;
        inline static Ref<IndexBuffer>  QuadIndexBuffer;

        static void Initialize();
        static void Shutdown();
    };
}