#include "Camera.h"
#include "Input.h"
using namespace DirectX;

Camera::Camera(float aspectRatio, DirectX::XMFLOAT3 position)
{
	transform = Transform();
	transform.SetPosition(position.x, position.y, position.z);

	transform.SetPitchYawRoll(0.6f, 0.0f, 0.0f);

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 position = transform.GetPosition();
	XMFLOAT3 forward = transform.GetForward();
	XMStoreFloat4x4(&view, XMMatrixLookToLH(XMLoadFloat3(&position), XMLoadFloat3(&forward), XMVectorSet(0, 1, 0, 0)));
}

void Camera::Update(DirectX::XMFLOAT3 playerPosition) {
	transform.SetPosition(playerPosition.x * 0.5f, 12, playerPosition.z * 0.3f - 15);
	UpdateViewMatrix();
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, aspectRatio, 0.1f, 1000.0f));
}

DirectX::XMFLOAT4X4 Camera::GetView()
{
	return view;
}

DirectX::XMFLOAT4X4 Camera::GetProjection()
{
	return projection;
}

DirectX::XMFLOAT3 Camera::GetPosition()
{
	return transform.GetPosition();
}

Transform* Camera::GetTransform()
{
	return &transform;
}
