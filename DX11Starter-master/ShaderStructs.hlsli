#ifndef __GGP_STRUCTS_INCLUDES__ // Each .hlsli file needs a unique identifier! 
#define __GGP_STRUCTS_INCLUDES__ 

struct VertexToPixel
{
	float4 screenPosition : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 worldPosition : POSITION;
	float3 tangent : TANGENT;
};

#endif