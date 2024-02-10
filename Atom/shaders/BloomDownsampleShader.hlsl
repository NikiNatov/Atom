#shadertype cs

#include "autogen/hlsl/DefaultLayout.hlsli"
#include "autogen/hlsl/BloomDownsampleParams.hlsli"

static DefaultLayoutStaticSamplers g_DefaultLayoutStaticSamplers = CreateDefaultLayoutStaticSamplers();
static BloomDownsampleParams g_BloomDownsampleParams = CreateBloomDownsampleParams();

[RootSignature(DefaultLayout_Compute)]
[numthreads(1, 1, 1)]
void CSMain(uint2 ThreadID : SV_DispatchThreadID)
{
    float targetWidth, targetHeight;
    g_BloomDownsampleParams.DownsampledTexture.GetDimensions(targetWidth, targetHeight);
    
    float2 targetTexelSize = 1.0f / float2(targetWidth, targetHeight);
    float2 texCoords = targetTexelSize * (ThreadID.xy);
    
    if (g_BloomDownsampleParams.DownsampledMipLevel == 0)
    {
        float4 color = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, texCoords, 0);
        g_BloomDownsampleParams.DownsampledTexture[ThreadID.xy] = color;
        return;
    }

    // Down sampling is done based on the method used in "Next-gen post processing in Call of Duty" presentation, ACM Siggraph 2014
    // https://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare/
	// a - b - c
	// - j - k -
	// d - e - f
	// - l - m -
	// g - h - i
	// 'e' = current texel
    
    float3 a = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x - 2 * targetTexelSize.x, texCoords.y + 2 * targetTexelSize.y), 0).rgb;
    float3 b = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x,                         texCoords.y + 2 * targetTexelSize.y), 0).rgb;
    float3 c = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x + 2 * targetTexelSize.x, texCoords.y + 2 * targetTexelSize.y), 0).rgb;

    float3 d = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x - 2 * targetTexelSize.x, texCoords.y), 0).rgb;
    float3 e = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x,                         texCoords.y), 0).rgb;
    float3 f = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x + 2 * targetTexelSize.x, texCoords.y), 0).rgb;

    float3 g = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x - 2 * targetTexelSize.x, texCoords.y - 2 * targetTexelSize.y), 0).rgb;
    float3 h = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x,                         texCoords.y - 2 * targetTexelSize.y), 0).rgb;
    float3 i = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x + 2 * targetTexelSize.x, texCoords.y - 2 * targetTexelSize.y), 0).rgb;

    float3 j = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x - targetTexelSize.x, texCoords.y + targetTexelSize.y), 0).rgb;
    float3 k = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x + targetTexelSize.x, texCoords.y + targetTexelSize.y), 0).rgb;
    float3 l = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x - targetTexelSize.x, texCoords.y - targetTexelSize.y), 0).rgb;
    float3 m = g_BloomDownsampleParams.SourceTexture.SampleLevel(g_DefaultLayoutStaticSamplers.LinearClampSampler, float2(texCoords.x + targetTexelSize.x, texCoords.y - targetTexelSize.y), 0).rgb;

    float3 downSampledColor = (j + k + m + l) * 0.5f / 4.0f;
    downSampledColor += (a + b + e + d) * 0.125f / 4.0f;
    downSampledColor += (b + c + f + e) * 0.125f / 4.0f;
    downSampledColor += (d + e + h + g) * 0.125f / 4.0f;
    downSampledColor += (e + f + i + h) * 0.125f / 4.0f;
    
    g_BloomDownsampleParams.DownsampledTexture[ThreadID.xy] = float4(downSampledColor, 1.0f);
}