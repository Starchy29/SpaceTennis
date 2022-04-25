#include "ShaderStructs.hlsli"
#include "Lighting.hlsli"

cbuffer ExternalData : register(b0) {
	float4 colorTint;
	float3 cameraPosition;
	float roughness;
	float uvScale;
	float3 ambient;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
}

Texture2D Albedo : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
SamplerState DefaultSampler : register(s0);

// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	float3 unpackedNormal = NormalMap.Sample(DefaultSampler, input.uv).rgb * 2 - 1;

	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);
	input.tangent = normalize(input.tangent - input.normal * dot(input.tangent, input.normal)); // Gram-Schmidt assumes T&N are normalized!
	float3 biTangent = cross(input.tangent, input.normal);
	float3x3 TBN = float3x3(input.tangent, biTangent, input.normal);
	input.normal = mul(unpackedNormal, TBN); // Note multiplication order!

	input.uv.x *= uvScale;
	input.uv.y *= uvScale;
	float3 view = normalize(cameraPosition - input.worldPosition);

	float4 surfaceColor = pow(Albedo.Sample(DefaultSampler, input.uv).rgba * colorTint, 2.2f);
	float roughness = RoughnessMap.Sample(DefaultSampler, input.uv).r;
	float metalness = MetalnessMap.Sample(DefaultSampler, input.uv).r;
	float3 specularColor = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, metalness);

	float4 mainDirectional = Directional(directionalLight1, view, input.normal, roughness, surfaceColor, specularColor, metalness);
	float4 backLeftDir = Directional(directionalLight2, view, input.normal, roughness, surfaceColor, specularColor, metalness);
	float4 backRightDir = Directional(directionalLight3, view, input.normal, roughness, surfaceColor, specularColor, metalness);
	float4 totalColor = backLeftDir + backRightDir + mainDirectional + float4(ambient, 1) * surfaceColor;
	return float4(pow(totalColor, 1.0f / 2.2f).rgb, 1);
}