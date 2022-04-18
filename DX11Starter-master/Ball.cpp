#include "Ball.h"

Ball::Ball(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) : Entity(mesh, material)
{
}

void Ball::Hit(Vector3 hit, bool fromPlayer)
{
	playerHit = fromPlayer;
	hasBounced = false;
}

void Ball::Update(float deltaTime)
{
	float minY = 0.5f; // floor height

	// check for bounce


	transform.MoveAbsolute(velocity.x, velocity.y, velocity.z);
}
