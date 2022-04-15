struct VertexToPixel {
	float4 position : SV_POSITION;
	float3 sampleDir : DIRECTION;
};

TextureCube SkyBox : register(t0);
SamplerState DefaultSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
	return SkyBox.Sample(DefaultSampler, input.sampleDir);
}