#pragma once
#include "Vector3.h"
#include "Entity.h"

// the tennis ball
class Ball : public Entity
{
public:
	Ball(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	void Hit(Vector3 hit, bool fromPlayer);
	void Update(float deltaTime);
	bool IsActive();
	void Serve(DirectX::XMFLOAT3 playerPosition);

private:
	Vector3 velocity;
	bool playerHit; // who hit it last? player or opponent
	bool hasBounced; // if the ball has bounced once yet, meaning the next bounce ends the point
	bool active; // if the ball exists
};

