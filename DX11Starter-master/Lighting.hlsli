#ifndef __GGP_LIGHTING_INCLUDES__ // Each .hlsli file needs a unique identifier! 
#define __GGP_LIGHTING_INCLUDES__ 

#define LIGHT_TYPE_DIRECTIONAL 0 
#define LIGHT_TYPE_POINT  1 
#define LIGHT_TYPE_SPOT  2
#define MAX_SPECULAR_EXPONENT 256.0f

struct Light {
	int type;
	float3 direction;
	float range;
	float3 position;
	float intensity;
	float3 color;
	float spotFallOff;
	float3 padding;
};

static const float F0_NON_METAL = 0.04f;
static const float MIN_ROUGHNESS = 0.0000001f;
static const float PI = 3.14159265359f;

// GGX (Trowbridge-Reitz)
// a - Roughness
// h - Half vector
// n - Normal
// D(h, n) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float SpecDistribution(float3 n, float3 h, float roughness) {
	// Pre-calculations
	float NdotH = saturate(dot(n, h));
	float NdotH2 = NdotH * NdotH;
	float a = roughness * roughness;
	float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	float denomToSquare = NdotH2 * (a2 - 1) + 1;
	// Can go to zero if roughness is 0 and NdotH is 1; MIN_ROUGHNESS helps here

	// Final value
	return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approx.
// v - View vector
// h - Half vector
// f0 - Value when l = n (full specular color)
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 Fresnel(float3 v, float3 h, float3 f0) {
	// Pre-calculations
	float VdotH = saturate(dot(v, h));

	// Final value
	return f0 + (1 - f0) * pow(1 - VdotH, 5);
}

// Geometric Shadowing - Schlick-GGX (based on Schlick-Beckmann)
// - k is remapped to a / 2, roughness remapped to (r+1)/2
// n - Normal
// v - View vector
// G(l,v)
float GeometricShadowing(float3 n, float3 v, float roughness) {
	// End result of remapping:
	float k = pow(roughness + 1, 2) / 8.0f;
	float NdotV = saturate(dot(n, v));

	// Final value
	return NdotV / (NdotV * (1 - k) + k);
}

// Microfacet BRDF (Specular)
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - part of the denominator are canceled out by numerator (see below)
// D() - Spec Dist - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 specColor) {
	// Other vectors
	float3 h = normalize(v + l);

	// Grab various functions
	float D = SpecDistribution(n, h, roughness);
	float3 F = Fresnel(v, h, specColor);
	float G = GeometricShadowing(n, v, roughness) * GeometricShadowing(n, l, roughness);

	// Final formula
	// Denominator dot products partially canceled by G()!
	// See page 16: http://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notes.pdf
	return (D * F * G) / (4 * max(dot(n, v), dot(n, l)));
}

float3 DiffuseEnergyConserve(float3 diffuse, float3 specular, float metalness) {
	return diffuse * ((1 - saturate(specular)) * (1 - metalness));
}

float Attenuate(Light light, float3 worldPos) {
	float dist = distance(light.position, worldPos);
	float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
	return att * att;
}

float4 Diffuse(Light light, float3 normal)
{
	float3 directionToLight = -normalize(light.direction);
	float diffuse = saturate(dot(directionToLight, normal));
	return float4(light.color * diffuse, 1);
}

float4 Specular(Light light, float3 view, float3 normal, float roughness, float3 specColor) {
	return float4(MicrofacetBRDF(normal, -normalize(light.direction), view, roughness, specColor), 1);
}

float4 Directional(Light light, float3 view, float3 normal, float roughness, float4 colorTint, float3 specColor, float metalness) {
	float diffuse = Diffuse(light, normal);
	float3 specular = Specular(light, view, normal, roughness, specColor);
	float3 balancedDiff = DiffuseEnergyConserve(diffuse, specular, metalness);

	return float4( (balancedDiff * colorTint + specular) * light.intensity * light.color, 1);
}

float4 Point(Light light, float3 view, float3 normal, float roughness, float4 colorTint, float3 worldPosition, float3 specColor, float metalness) {
	light.direction = normalize(light.position - worldPosition);
	return Directional(light, view, normal, roughness, colorTint, specColor, metalness) * Attenuate(light, worldPosition);
}

#endif