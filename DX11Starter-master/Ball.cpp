#include "Ball.h"
#include "Game.h"

Ball::Ball(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) : Entity(mesh, material)
{
	playerHit = false;
	hasBounced = false;
	transform.SetPosition(0, 5, -6);
}

void Ball::Hit(Vector3 hit, bool fromPlayer)
{
	playerHit = fromPlayer;
	hasBounced = false;
	velocity = hit;
}

void Ball::Update(float deltaTime)
{
	float minY = 0.5f; // floor height

	// apply gravity
	if(deltaTime < 1.0f) {
		velocity.y -= 10.0f * deltaTime;
	}

	transform.MoveAbsolute(velocity.x * deltaTime, velocity.y * deltaTime, velocity.z * deltaTime);

	// check for bounce
	DirectX::XMFLOAT3 position = transform.GetPosition();
	if(position.y <= minY) {
		transform.SetPosition(position.x, minY, position.z);
		velocity.y *= -1;

		// check if bounced out of court
	}

	// temp: bounce off back wall
	if(position.z > Game::AREA_HALF_HEIGHT || position.z < -Game::AREA_HALF_HEIGHT) {
		velocity.z *= -1;
	}

	// check for net collision
}
