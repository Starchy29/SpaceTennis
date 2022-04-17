#pragma once
#include "Entity.h"
#include "Vector3.h"

class Player : public Entity
{
public:
	void Update(float dt);
	Player(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

private:
	Vector3 velocity;
};

