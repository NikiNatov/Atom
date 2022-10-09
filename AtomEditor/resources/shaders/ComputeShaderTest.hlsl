cbuffer TextureDimensions : register(b0)
{
	uint Width;
	uint Height;
};

RWTexture2D<float4> OutputTexture : register(u0);

[numthreads(32, 32, 1)]
void CSMain(uint3 ThreadID : SV_DispatchThreadID)
{
	OutputTexture[ThreadID.xy] = float4((float)ThreadID.x / (float)Width, (float)ThreadID.y / (float)Height, 0.0f, 1.0f);
}