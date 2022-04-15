#include "Sky.h"

Sky::Sky(std::shared_ptr<Mesh> mesh, Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState, Microsoft::WRL::ComPtr<ID3D11Device> device, std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV)
{
	this->mesh = mesh;
	this->samplerState = samplerState;
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
	this->textureSRV = textureSRV;

	D3D11_RASTERIZER_DESC rasterDescription = {};
	rasterDescription.FillMode = D3D11_FILL_SOLID;
	rasterDescription.CullMode = D3D11_CULL_FRONT;
	device.Get()->CreateRasterizerState(&rasterDescription, &rasterizerState);

	D3D11_DEPTH_STENCIL_DESC stencilDescription = {};
	stencilDescription.DepthEnable = true;
	stencilDescription.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device.Get()->CreateDepthStencilState(&stencilDescription, &depthStencilState);
}

void Sky::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera)
{
	context->RSSetState(rasterizerState.Get());
	context->OMSetDepthStencilState(depthStencilState.Get(), 0);

	vertexShader->SetMatrix4x4("view", camera->GetView());
	vertexShader->SetMatrix4x4("projection", camera->GetProjection());
	vertexShader->CopyAllBufferData();
	vertexShader->SetShader();

	pixelShader->SetSamplerState("DefaultSampler", samplerState);
	pixelShader->SetShaderResourceView("SkyBox", textureSRV);
	pixelShader->CopyAllBufferData();
	pixelShader->SetShader();

	mesh->Draw();

	context->RSSetState(nullptr);
	context->OMSetDepthStencilState(nullptr, 0);
}
