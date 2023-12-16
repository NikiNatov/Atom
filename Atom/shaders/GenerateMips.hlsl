#shadertype cs

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/GenerateMipsParams.hlsli"

static DefaultLayoutStaticSamplers g_DefaultLayoutStaticSamplers = CreateDefaultLayoutStaticSamplers();
static GenerateMipsParams g_GenerateMipsParams = CreateGenerateMipsParams();

[RootSignature(DefaultLayout_Compute)]
[numthreads(8, 8, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
	//ThreadID is the thread ID * the values from numthreads above and in this case correspond to the pixels location in number of pixels.
	//As a result texcoords (in 0-1 range) will point at the center between the 4 pixels used for the mipmap.
	float2 texCoords = g_GenerateMipsParams.TexelSize * (ThreadID.xy + 0.5);

	//The samplers linear interpolation will mix the four pixel values to the new pixels color
    float4 color = g_GenerateMipsParams.SrcTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, texCoords, g_GenerateMipsParams.TopMipLevel);

	//Write the final color into the destination texture.
	g_GenerateMipsParams.DstTexture[ThreadID.xy] = color;
}