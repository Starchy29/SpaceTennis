#pragma once
#include "Transform.h"

class Camera
{
public:
	Camera(float aspectRatio, DirectX::XMFLOAT3 position);
	void Update(float dt);
	void UpdateProjectionMatrix(float aspectRatio);

	DirectX::XMFLOAT4X4 GetView();
	DirectX::XMFLOAT4X4 GetProjection();
	DirectX::XMFLOAT3 GetPosition();

private:
	Transform transform;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;

	void UpdateViewMatrix();
};

