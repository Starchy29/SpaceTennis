#include "Transform.h"
#include <DirectXMath.h>
using namespace DirectX;

Transform::Transform()
{
	position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pitchYawRoll = XMFLOAT3(0.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());

	needsUpdate = false;
}

void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	needsUpdate = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	needsUpdate = true;
}

void Transform::SetPitchYawRoll(float pitch, float yaw, float roll)
{
	pitchYawRoll = XMFLOAT3(pitch, yaw, roll);
	needsUpdate = true;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	XMVECTOR shift = XMVectorSet(x, y, z, 0);
	XMVECTOR mathPos = XMLoadFloat3(&position);
	XMStoreFloat3(&position, mathPos + shift);

	needsUpdate = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	XMVECTOR mathPos = XMLoadFloat3(&position);
	XMVECTOR shift = XMVector3Rotate(XMVectorSet(x, y, z, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll)));
	XMStoreFloat3(&position, mathPos + shift);

	needsUpdate = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	XMVECTOR spin = XMVectorSet(pitch, yaw, roll, 0);
	XMVECTOR mathPYR = XMLoadFloat3(&pitchYawRoll);
	XMStoreFloat3(&pitchYawRoll, spin + mathPYR);

	needsUpdate = true;
}

void Transform::Scale(float x, float y, float z)
{
	XMVECTOR growth = XMVectorSet(x, y, z, 0);
	XMVECTOR mathScale = XMLoadFloat3(&scale);
	XMStoreFloat3(&scale, growth * mathScale);

	needsUpdate = true;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return pitchYawRoll;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if(needsUpdate) {
		UpdateMatrices();
	}

	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	if(needsUpdate) {
		UpdateMatrices();
	}

	return worldInverseTranspose;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVector3Rotate(XMVectorSet(1, 0, 0, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll))));
	return result;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVector3Rotate(XMVectorSet(0, 1, 0, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll))));
	return result;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll))));
	return result;
}

void Transform::UpdateMatrices()
{
	XMMATRIX translMat = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
	XMMATRIX rotMat = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&pitchYawRoll));
	XMMATRIX scaleMat = XMMatrixScalingFromVector(XMLoadFloat3(&scale));

	XMMATRIX world = scaleMat * rotMat * translMat;

	XMStoreFloat4x4(&worldMatrix, world);
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(world)));

	needsUpdate = false;
}
