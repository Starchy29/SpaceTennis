#include "Vector3.h"
#include <math.h>

Vector3::Vector3(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}


Vector3::Vector3(const Vector3& other)
{
	this->x = other.x;
	this->y = other.y;
	this->z = other.z;
}

Vector3::Vector3() {
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

void Vector3::SetLength(float length) {
	float oldLength = Length();
	if(oldLength == 0.0f) {
		return;
	}

	x = x / oldLength * length;
	y = y / oldLength * length;
	z = z / oldLength * length;
}

void Vector3::Add(Vector3 other) {
	x += other.x;
	y += other.y;
	z += other.z;
}

void Vector3::Add(float length) {
	Vector3 copy = Vector3(*this);
	copy.SetLength(length);
	x += copy.x;
	y += copy.y;
	z += copy.z;
}

void Vector3::Add(float x, float y, float z) {
	this->x += x;
	this->y += y;
	this->z += z;
}

float Vector3::Dot(Vector3 other) {
	return x * other.x + y * other.y + z * other.z;
}

bool Vector3::Equals(Vector3 other) {
	return x == other.x && y == other.y && z == other.z;
}

float Vector3::Length() {
	return sqrt(x * x + y * y + z * z);
}