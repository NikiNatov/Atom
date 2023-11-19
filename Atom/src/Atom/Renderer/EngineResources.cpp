#include "atompch.h"
#include "EngineResources.h"

#include "Atom/Renderer/Renderer.h"
#include "Atom/Renderer/Device.h"
#include "Atom/Renderer/ShaderLibrary.h"
#include "Atom/Renderer/PipelineLibrary.h"

#include "autogen/cpp/BRDFParams.h"

namespace Atom
{
    // -----------------------------------------------------------------------------------------------------------------------------
    void EngineResources::Initialize()
    {
        //===============================================================================================================//
        //                                                  Textures                                                     //
        //===============================================================================================================//
        {
            TextureDescription errorTextureDesc;
            errorTextureDesc.Width = 1;
            errorTextureDesc.Height = 1;
            errorTextureDesc.Format = TextureFormat::RGBA8;
            errorTextureDesc.MipLevels = 1;

            ErrorTexture = CreateRef<Texture>(errorTextureDesc, "EngineResources::ErrorTexture");

            const byte errorTextureData[] = { 0xFF, 0x00, 0xFF, 0xFF };
            Renderer::UploadTextureData(ErrorTexture, errorTextureData);
        }

        {
            TextureDescription blackTextureDesc;
            blackTextureDesc.Width = 1;
            blackTextureDesc.Height = 1;
            blackTextureDesc.Format = TextureFormat::RGBA8;
            blackTextureDesc.MipLevels = 1;

            BlackTexture = CreateRef<Texture>(blackTextureDesc, "EngineResources::BlackTexture");

            blackTextureDesc.ArraySize = 6;
            blackTextureDesc.Flags |= TextureFlags::CubeMap;

            BlackTextureCube = CreateRef<Texture>(blackTextureDesc, "EngineResources::BlackTextureCube");

            const byte blackTextureData[] = { 0x00, 0x00, 0x00, 0xFF };
            Renderer::UploadTextureData(BlackTexture, blackTextureData);

            for (u32 face = 0; face < 6; face++)
                Renderer::UploadTextureData(BlackTextureCube, blackTextureData, 0, face);
        }

        {
            ComputePipelineDescription pipelineDesc;
            pipelineDesc.Shader = ShaderLibrary::Get().Get<ComputeShader>("BRDFShader");

            Ref<ComputePipeline> brdfPipeline = PipelineLibrary::Get().LoadComputePipeline(pipelineDesc, "BRDFPipeline");

            TextureDescription brdfTextureDesc;
            brdfTextureDesc.Width = 256;
            brdfTextureDesc.Height = 256;
            brdfTextureDesc.Format = TextureFormat::RG16F;
            brdfTextureDesc.MipLevels = 1;
            brdfTextureDesc.Flags = TextureFlags::UnorderedAccess | TextureFlags::ShaderResource;
            brdfTextureDesc.InitialState = ResourceState::UnorderedAccess;

            BRDFTexture = CreateRef<Texture>(brdfTextureDesc, "EngineResources::BRDFTexture");

            SIG::BRDFParams brdfParams;
            brdfParams.SetBRDFTexture(BRDFTexture.get());
            brdfParams.Compile();

            GPUDescriptorHeap* resourceHeap = Device::Get().GetGPUDescriptorHeap(DescriptorHeapType::ShaderResource);
            CommandQueue* computeQueue = Device::Get().GetCommandQueue(CommandQueueType::Compute);
            Ref<CommandBuffer> computeCmdBuffer = computeQueue->GetCommandBuffer();
            computeCmdBuffer->Begin();
            computeCmdBuffer->SetComputePipeline(brdfPipeline.get());
            computeCmdBuffer->SetDescriptorHeaps(resourceHeap, nullptr);
            computeCmdBuffer->SetComputeDescriptorTables(ShaderBindPoint::Instance, brdfParams.GetResourceTable());
            computeCmdBuffer->Dispatch(glm::max(brdfTextureDesc.Width / 32, 1u), glm::max(brdfTextureDesc.Height / 32, 1u), 1);
            computeCmdBuffer->TransitionResource(BRDFTexture.get(), ResourceState::Common);
            computeCmdBuffer->End();
            computeQueue->ExecuteCommandList(computeCmdBuffer);
        }

        //===============================================================================================================//
        //                                                  Materials                                                    //
        //===============================================================================================================//
        DefaultMaterial = CreateRef<Material>(ShaderLibrary::Get().Get<GraphicsShader>("MeshPBRShader"), MaterialFlags::DepthTested | MaterialFlags::TwoSided);
        DefaultMaterial->SetUniform("AlbedoColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        DefaultMaterial->SetUniform("Metalness", 0.5f);
        DefaultMaterial->SetUniform("Roughness", 0.5f);

        DefaultMaterialAnimated = CreateRef<Material>(ShaderLibrary::Get().Get<GraphicsShader>("MeshPBRAnimatedShader"), MaterialFlags::DepthTested | MaterialFlags::TwoSided);
        DefaultMaterialAnimated->SetUniform("AlbedoColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        DefaultMaterialAnimated->SetUniform("Metalness", 0.5f);
        DefaultMaterialAnimated->SetUniform("Roughness", 0.5f);

        ErrorMaterial = CreateRef<Material>(ShaderLibrary::Get().Get<GraphicsShader>("MeshPBRShader"), MaterialFlags::DepthTested | MaterialFlags::TwoSided);
        ErrorMaterial->SetUniform("AlbedoColor", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        ErrorMaterial->SetUniform("Metalness", 0.0f);
        ErrorMaterial->SetUniform("Roughness", 1.0f);

        ErrorMaterialAnimated = CreateRef<Material>(ShaderLibrary::Get().Get<GraphicsShader>("MeshPBRAnimatedShader"), MaterialFlags::DepthTested | MaterialFlags::TwoSided);
        ErrorMaterialAnimated->SetUniform("AlbedoColor", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        ErrorMaterialAnimated->SetUniform("Metalness", 0.0f);
        ErrorMaterialAnimated->SetUniform("Roughness", 1.0f);

        //===============================================================================================================//
        //                                                  Samplers                                                     //
        //===============================================================================================================//
        LinearClampSampler = CreateRef<TextureSampler>(TextureFilter::Linear, TextureWrap::Clamp);
        LinearRepeatSampler = CreateRef<TextureSampler>(TextureFilter::Linear, TextureWrap::Repeat);
        NearestClampSampler = CreateRef<TextureSampler>(TextureFilter::Nearest, TextureWrap::Clamp);
        NearestRepeatSampler = CreateRef<TextureSampler>(TextureFilter::Nearest, TextureWrap::Repeat);
        AnisotropicClampSampler = CreateRef<TextureSampler>(TextureFilter::Anisotropic, TextureWrap::Clamp);
        AnisotropicRepeatSampler = CreateRef<TextureSampler>(TextureFilter::Anisotropic, TextureWrap::Repeat);

        //===============================================================================================================//
        //                                                   Buffers                                                     //
        //===============================================================================================================//
        {
            f32 quadVertexData[] = {
                // x     y     z     u     v
                -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
                 1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
                 1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
                -1.0f,  1.0f, 0.0f, 0.0f, 0.0f
            };

            BufferDescription vbDesc;
            vbDesc.ElementCount = _countof(quadVertexData) / 5;
            vbDesc.ElementSize = sizeof(f32) * 5;
            vbDesc.IsDynamic = false;

            QuadVertexBuffer = CreateRef<VertexBuffer>(vbDesc, "EngineResources::QuadVertexBuffer");
            Renderer::UploadBufferData(QuadVertexBuffer, quadVertexData);
        }

        {
            u16 quadIndices[] = { 0, 1, 2, 2, 3, 0 };

            BufferDescription ibDesc;
            ibDesc.ElementCount = _countof(quadIndices);
            ibDesc.ElementSize = sizeof(u16);
            ibDesc.IsDynamic = false;

            QuadIndexBuffer = CreateRef<IndexBuffer>(ibDesc, IndexBufferFormat::U16, "EngineResources::QuadIndexBuffer");
            Renderer::UploadBufferData(QuadIndexBuffer, quadIndices);
        }

        // Wait for all upload operations to complete before we continue
        Device::Get().GetCommandQueue(CommandQueueType::Copy)->Flush();
        Device::Get().GetCommandQueue(CommandQueueType::Compute)->Flush();
    }

    // -----------------------------------------------------------------------------------------------------------------------------
    void EngineResources::Shutdown()
    {
        // Textures
        BlackTexture = nullptr;
        BlackTextureCube = nullptr;
        ErrorTexture = nullptr;
        BRDFTexture = nullptr;

        // Materials
        DefaultMaterial = nullptr;
        DefaultMaterialAnimated = nullptr;
        ErrorMaterial = nullptr;
        ErrorMaterialAnimated = nullptr;

        // Samplers
        LinearClampSampler = nullptr;
        LinearRepeatSampler = nullptr;
        NearestClampSampler = nullptr;
        NearestRepeatSampler = nullptr;
        AnisotropicClampSampler = nullptr;
        AnisotropicRepeatSampler = nullptr;

        // Buffers
        QuadVertexBuffer = nullptr;
        QuadIndexBuffer = nullptr;
    }
}
