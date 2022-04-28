#include "Player.h"
#include "Input.h"
#include "Transform.h"
#include "Game.h"

using namespace DirectX;

void Player::Update(float dt, Ball* ball)
{
	Input& input = Input::GetInstance();
	float maxSpeed = 13.0f;
	float displacement = 5.0f * dt;
	float acceleration = 90.0f * dt;
	float minY = 1.5f;

	// accelerate from input
	Vector3 moveDirection = Vector3();
	if (input.KeyDown(VK_UP)) { moveDirection.z += 1; }
	if (input.KeyDown(VK_DOWN)) { moveDirection.z -= 1; }
	if (input.KeyDown(VK_LEFT)) { 
		moveDirection.x -= 1; 
		if(!input.KeyDown('W') && !input.KeyRelease('W')) { // don't release on the frame the player swings either
			facingRight = false;
		}
	}
	if (input.KeyDown(VK_RIGHT)) { 
		moveDirection.x += 1; 
		if(!input.KeyDown('W') && !input.KeyRelease('W')) {
			facingRight = true;
		}
	}

	if(!moveDirection.Equals(Vector3())) {
		moveDirection.SetLength(acceleration);
		velocity.Add(moveDirection);
	}

	if(transform.GetPosition().y <= minY) {
		// apply friction
		float friction = 40.0f;
		Vector3 lastVel = velocity;
		velocity.Add(-friction * dt);

		// check if passed 0
		if(lastVel.Dot(velocity) < 0) {
			velocity.SetLength(0);
		}
	} else {
		// apply gravity in air
		if(input.KeyDown(VK_SPACE)) {
			velocity.y -= 20 * dt; // extend jump height and fall slower
		} else {
			velocity.y -= 60 * dt; // regular gravity
		}
	}
	
	// cap speed
	Vector3 horVel = Vector3(velocity.x, 0, velocity.z);
	if(input.KeyDown('W')) {
		// move slower when holding a swing
		maxSpeed /= 3.0f;
	}
	if(horVel.Length() > maxSpeed) {
		horVel.SetLength(maxSpeed);
		velocity = Vector3(horVel.x, velocity.y, horVel.z);
	}

	// jump
	if(transform.GetPosition().y <= minY && input.KeyDown(VK_SPACE)) { 
		velocity.y = 20;  // jump velocity
	}

	// move
	transform.MoveRelative(velocity.x * dt, velocity.y * dt, velocity.z * dt);

	// lock player in court
	DirectX::XMFLOAT3 position = transform.GetPosition();
	if(position.y < minY) { // floor
		transform.SetPosition(position.x, minY, position.z);
		position = transform.GetPosition();
	}
	if(position.x < -Game::AREA_HALF_WIDTH) { // left wall
		transform.SetPosition(-Game::AREA_HALF_WIDTH, position.y, position.z);
		position = transform.GetPosition();
	}
	else if(position.x > Game::AREA_HALF_WIDTH) { // right wall
		transform.SetPosition(Game::AREA_HALF_WIDTH, position.y, position.z);
		position = transform.GetPosition();
	}
	if(position.z > -1.0f) { // net
		transform.SetPosition(position.x, position.y, -1.0f);
	}
	else if(position.z < -Game::AREA_HALF_HEIGHT) { // back wall
		transform.SetPosition(position.x, position.y, -Game::AREA_HALF_HEIGHT);
	}

	// swing at ball
	if(input.KeyRelease('W') && ball != nullptr) {
		swingCooldown = 1.0f;

		float reach = 2.0f;
		if(!facingRight) {
			reach *= -1; // swing left instead
		}

		// check sphere collision
		XMFLOAT3 ballPosition = ball->GetTransform()->GetPosition();
		float dx = ballPosition.x - position.x - reach;
		float dy = ballPosition.y - position.y;
		float dz = ballPosition.z - position.z;
		float distSquared = dx * dx + dy * dy + dz * dz;
		if(distSquared < 8) {
			// hit ball
			float aimer = 0.0f;
			if(input.KeyDown(VK_RIGHT)) {
				aimer = 4.0f;
			}
			if(input.KeyDown(VK_LEFT)) {
				aimer = -4.0f;
			}

			aimer += dz * 3 * (facingRight ? -1 : 1);

			if(position.y <= minY) {
				ball->Hit(Vector3(aimer, 8, 12), true); // ground stroke
			} else {
				ball->Hit(Vector3(aimer, -1.5f * position.y, -3 * position.z + (position.z > -3.0f ? 2.0f : 0.0f)), true); // spike midair
			}
		}
	}

	if(swingCooldown > 0) {
		swingCooldown -= dt;
	}

	// make racket follow player
	racketHandle->GetTransform()->SetPosition((facingRight ? 1 : -1) * 0.9f + position.x, position.y, position.z);
	racketHead->GetTransform()->SetPosition((facingRight ? 1 : -1) * 1.5f + position.x, position.y, position.z);
}

Player::Player(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) : Entity(mesh, material) {
	velocity = Vector3(0.0f, 0.0f, 0.0f);
	transform.SetPosition(0.0f, 1.5f, 0.0f);
	facingRight = true;
	swingCooldown = 0;
}
