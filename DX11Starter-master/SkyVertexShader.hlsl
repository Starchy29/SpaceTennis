cbuffer ExternalData : register(b0) {
	matrix view;
	matrix projection;
}

struct VertexShaderInput
{
	float3 localPosition: POSITION; // XYZ position
	float3 normal: NORMAL;
	float3 tangent: TANGENT;
	float2 uv: TEXCOORD;
};

struct VertexToPixel {
	float4 position : SV_POSITION;
	float3 sampleDir : DIRECTION;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;
	matrix viewNoTransl = view;
	viewNoTransl._14 = 0;
	viewNoTransl._24 = 0;
	viewNoTransl._34 = 0;

	output.position = mul(projection, mul(viewNoTransl, input.localPosition));
	output.position.z = output.position.w; // set depth to as far as possible
	output.sampleDir = input.localPosition;

	return output;
}