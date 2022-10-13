cbuffer GenarateMipsCB : register(b0)
{
	float2 TexelSize;
	uint TopMipLevel;
}

Texture2D<float4> SrcTexture : register(t0);
SamplerState BilinearClamp : register(s0);

RWTexture2D<float4> DstTexture : register(u0);

[numthreads(8, 8, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
	//ThreadID is the thread ID * the values from numthreads above and in this case correspond to the pixels location in number of pixels.
	//As a result texcoords (in 0-1 range) will point at the center between the 4 pixels used for the mipmap.
	float2 texCoords = TexelSize * (ThreadID.xy + 0.5);

	//The samplers linear interpolation will mix the four pixel values to the new pixels color
	float4 color = SrcTexture.SampleLevel(BilinearClamp, texCoords, TopMipLevel);

	//Write the final color into the destination texture.
	DstTexture[ThreadID.xy] = color;
}