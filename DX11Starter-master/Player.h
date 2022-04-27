#pragma once
#include "Entity.h"
#include "Vector3.h"
#include "Ball.h"

class Player : public Entity
{
public:
	void Update(float dt, Ball* ball);
	Player(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	Entity* racketHead;
	Entity* racketHandle;

private:
	Vector3 velocity;
	bool facingRight; // which side the racket is on
	float swingCooldown;
};

