#pragma once
#include "Transform.h"

class Camera
{
public:
	Camera(float aspectRatio, DirectX::XMFLOAT3 position);
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();

	void Update(DirectX::XMFLOAT3 playerPosition);

	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	DirectX::XMFLOAT3 GetPosition();
	Transform* GetTransform();

private:
	Transform transform;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};