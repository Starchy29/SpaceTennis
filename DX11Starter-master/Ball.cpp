#include "Ball.h"
#include "Game.h"

Ball::Ball(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) : Entity(mesh, material)
{
	playerHit = false;
	hasBounced = false;
	active = false;
	transform.SetScale(0.5f, 0.5f, 0.5f);
}

void Ball::Hit(Vector3 hit, bool fromPlayer)
{
	playerHit = fromPlayer;
	hasBounced = false;
	velocity = hit;
}

// returns which player got a point: >0 player, <0 enemy, 0 no one
int Ball::Update(float deltaTime)
{
	int result = 0;
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

		if(hasBounced) {
			// point ends from double bounce
			active = false;
			result = (playerHit ? 1 : -1);
		}
		else {
			hasBounced = true;

			// check if bounced out of court
			float buffer = 0.5f;
			if(position.x < -Game::COURT_HALF_WIDTH - buffer// out left
				|| position.x > Game::COURT_HALF_WIDTH + buffer // out right
				|| position.z < -Game::COURT_HALF_HEIGHT - buffer// out back
				|| position.z > Game::COURT_HALF_HEIGHT + buffer// out front
			) {
				active = false;
				result = (!playerHit ? 1 : -1);
			}
			
			if(playerHit && position.z < 0 || !playerHit && position.z > 0) { // land in own court
				active = false;
				result = (!playerHit ? 1 : -1);
			}
		}
	}

	// check for net collision
	if(abs(position.z) < 0.3f && abs(position.y) < 3.3f) {
		active = false;
		result = (!playerHit ? 1 : -1);
	}

	return result;
}

bool Ball::IsActive() {
	return active;
}

void Ball::Serve(DirectX::XMFLOAT3 playerPosition)
{
	transform.SetPosition(playerPosition.x + 1.5f, playerPosition.y + 2.5f, playerPosition.z);
	active = true;
	velocity = Vector3(0, 12.0f, 0);

	// make player lose if they miss the serve
	playerHit = false;
	hasBounced = true;
}
