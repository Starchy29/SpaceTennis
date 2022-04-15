#include "Camera.h"
#include "Input.h"
using namespace DirectX;

Camera::Camera(float aspectRatio, DirectX::XMFLOAT3 position)
{
	transform = Transform();
	transform.SetPosition(position.x, position.y, position.z);

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::Update(float dt)
{
	Input& input = Input::GetInstance();

	float speed = 10.0f;

	// keyboard movement
	if(input.KeyDown('W')) { transform.MoveRelative(0, 0, speed * dt); }
	if(input.KeyDown('S')) { transform.MoveRelative(0, 0, -speed * dt); }
	if(input.KeyDown('A')) { transform.MoveRelative(-speed * dt, 0, 0); }
	if(input.KeyDown('D')) { transform.MoveRelative(speed * dt, 0, 0); }
	if(input.KeyDown(VK_SPACE)) { transform.MoveAbsolute(0, speed * dt, 0); }
	if(input.KeyDown(VK_LSHIFT)) { transform.MoveAbsolute(0, -speed * dt, 0); }

	// mouse aim
	float sensitivity = 0.5f;
	if(input.MouseLeftDown()) {
		int cursorMovementX = input.GetMouseXDelta();
		int cursorMovementY = input.GetMouseYDelta();
		transform.Rotate(sensitivity * cursorMovementY * dt, sensitivity * cursorMovementX * dt, 0);

		// limit vertical to straight up and down
		if(transform.GetPitchYawRoll().x > XM_PIDIV2) {
			transform.SetPitchYawRoll(XM_PIDIV2, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z);
		}
		else if(transform.GetPitchYawRoll().x < -XM_PIDIV2) {
			transform.SetPitchYawRoll(-XM_PIDIV2, transform.GetPitchYawRoll().y, transform.GetPitchYawRoll().z);
		}
	}

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 position = transform.GetPosition();
	XMFLOAT3 forward = transform.GetForward();
	XMStoreFloat4x4(&view, XMMatrixLookToLH(XMLoadFloat3(&position), XMLoadFloat3(&forward), XMVectorSet(0, 1, 0, 0)));
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
