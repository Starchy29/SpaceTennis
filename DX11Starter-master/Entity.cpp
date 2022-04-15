#include "Entity.h"
#include "Camera.h"

using namespace DirectX;

Entity::Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	this->mesh = mesh;
	this->material = material;
}

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera)
{
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	vs->SetMatrix4x4("world", transform.GetWorldMatrix());
	vs->SetMatrix4x4("worldInverseTranspose", transform.GetWorldInverseTransposeMatrix());
	vs->SetMatrix4x4("view", camera->GetView()); 
	vs->SetMatrix4x4("projection", camera->GetProjection()); 

	vs->CopyAllBufferData();

	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();
	ps->SetFloat4("colorTint", material->GetTint());
	ps->SetFloat3("cameraPosition", camera->GetPosition());
	ps->SetFloat("roughness", material->GetRoughness());
	ps->SetFloat("uvScale", material->GetUVScale());

	for (auto& t : material->GetTextureSRVs()) { ps->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : material->GetSamplers()) { ps->SetSamplerState(s.first.c_str(), s.second); }

	ps->CopyAllBufferData();

	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	mesh->Draw();
}

Transform* Entity::GetTransform()
{
	return &transform;
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
	return mesh;
}

std::shared_ptr<Material> Entity::GetMaterial()
{
	return material;
}
