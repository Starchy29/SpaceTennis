#include "ShaderStructs.hlsli"

cbuffer ExternalData : register(b0) { 
	matrix world;
	matrix worldInverseTranspose;
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

VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Multiply the three matrices together first  
	matrix wvp = mul(projection, mul(view, world));  
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
	output.normal = mul((float3x3)worldInverseTranspose, input.normal);
	output.tangent = mul((float3x3)worldInverseTranspose, input.tangent);
	output.worldPosition = mul(world, float4(input.localPosition, 1)).xyz;

	// Pass the color through 
	// - The values will be interpolated per-pixel by the rasterizer
	// - We don't need to alter it here, but we do need to send it to the pixel shader
	//output.color = colorTint;
	output.uv = input.uv;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}