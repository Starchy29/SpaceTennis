#include "Player.h"
#include "Input.h"
#include "Transform.h"

void Player::Update(float dt)
{
	Input& input = Input::GetInstance();
	float maxSpeed = 13.0f;
	float displacement = 5.0f * dt;
	float acceleration = 90.0f * dt;

	// accelerate from input
	Vector3 moveDirection = Vector3();
	if (input.KeyDown('W')) { moveDirection.z += 1; }
	if (input.KeyDown('S')) { moveDirection.z -= 1; }
	if (input.KeyDown('A')) { moveDirection.x -= 1; }
	if (input.KeyDown('D')) { moveDirection.x += 1; }

	if(!moveDirection.Equals(Vector3())) {
		moveDirection.SetLength(acceleration);
		velocity.Add(moveDirection);
	}

	// apply friction
	float friction = 0.7f;
	Vector3 lastVel = velocity;
	velocity.Add(-friction);

	// check if passed 0
	if(lastVel.Dot(velocity) < 0) {
		velocity.SetLength(0);
	}

	// cap speed
	if(velocity.Length() > maxSpeed) {
		velocity.SetLength(maxSpeed);
	}

	//if (input.KeyDown(VK_SPACE)) { transform.MoveAbsolute(0, speed * dt, 0); }
	//if (input.KeyDown(VK_LSHIFT)) { transform.MoveAbsolute(0, -speed * dt, 0); }

	// move
	transform.MoveRelative(velocity.x * dt, velocity.y * dt, velocity.z * dt);
}

Player::Player(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) : Entity(mesh, material) {
	velocity = Vector3(0.0f, 0.0f, 0.0f);
}
