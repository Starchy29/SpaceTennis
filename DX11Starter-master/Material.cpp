#include "Material.h"

Material::Material(DirectX::XMFLOAT4 tint, std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader, float roughness)
{
    this->tint = tint;
    this->pixelShader = pixelShader;
    this->vertexShader = vertexShader;
    this->roughness = roughness;
    this->uvScale = 1.0f;
}

DirectX::XMFLOAT4 Material::GetTint()
{
    return tint;
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
    return pixelShader;
}

void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
    textureSRVs.insert({ name, srv });
}

void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState)
{
    samplers.insert({ name, samplerState });
}

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> Material::GetTextureSRVs()
{
    return textureSRVs;
}

std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> Material::GetSamplers()
{
    return samplers;
}

void Material::SetUVScale(float value)
{
    uvScale = value;
}

float Material::GetUVScale()
{
    return uvScale;
}

float Material::GetRoughness()
{
    return roughness;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
    return vertexShader;
}
