#pragma once
class Vector3
{
public:
	Vector3(float x, float y, float z);
	Vector3();
	Vector3(const Vector3& other);
	void SetLength(float length);
	void Add(Vector3 other);
	void Add(float length);
	void Add(float x, float y, float z);
	float Dot(Vector3 other);
	bool Equals(Vector3 other);
	float Length();

	float x;
	float y;
	float z;
};