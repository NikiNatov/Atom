#shadertype cs

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/BloomUpsampleParams.hlsli"

static DefaultLayoutStaticSamplers g_DefaultLayoutStaticSamplers = CreateDefaultLayoutStaticSamplers();
static BloomUpsampleParams g_BloomUpsampleParams = CreateBloomUpsampleParams();

[RootSignature(DefaultLayout_Compute)]
[numthreads(8, 8, 1)]
void CSMain(uint2 ThreadID : SV_DispatchThreadID)
{
    float upsampledWidth, upsampledHeight;
    g_BloomUpsampleParams.UpsampledTexture.GetDimensions(upsampledWidth, upsampledHeight);
    
    float2 upsampleTexelSize = 1.0f / float2(upsampledWidth, upsampledHeight);
    float2 texCoords = upsampleTexelSize * (ThreadID.xy);
    
    // Upsampling is done based on the method used in "Next-gen post processing in Call of Duty" presentation, ACM Siggraph 2014
    // https://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare/
	// a - b - c
	// d - e - f
	// g - h - i
	// 'e' = current texel
    
    float radius = g_BloomUpsampleParams.FilterRadius;
    float3 a = g_BloomUpsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x - radius, texCoords.y + radius), 0).rgb;
    float3 b = g_BloomUpsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x,          texCoords.y + radius), 0).rgb;
    float3 c = g_BloomUpsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x + radius, texCoords.y + radius), 0).rgb;
    
    float3 d = g_BloomUpsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x - radius, texCoords.y), 0).rgb;
    float3 e = g_BloomUpsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x,          texCoords.y), 0).rgb;
    float3 f = g_BloomUpsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x + radius, texCoords.y), 0).rgb;
    
    float3 g = g_BloomUpsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x - radius, texCoords.y - radius), 0).rgb;
    float3 h = g_BloomUpsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x,          texCoords.y - radius), 0).rgb;
    float3 i = g_BloomUpsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x + radius, texCoords.y - radius), 0).rgb;
    
    // Apply a 3x3 tent filter:
    float3 upsampledColor = e * 4.0f;
    upsampledColor += (b + d + f + h) * 2.0f;
    upsampledColor += (a + c + g + i);
    upsampledColor *= 1.0f / 16.0f;
    
    g_BloomUpsampleParams.UpsampledTexture[ThreadID.xy] += float4(upsampledColor, 1.0f);
}