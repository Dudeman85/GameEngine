#pragma once
#include <engine/Application.h>
#include <cmath>

extern ECS ecs;

using namespace engine;

//Player Component
struct Player
{
	float moveSpeed = 500.f;
	
	
};

//Player Controller requires Player, Sprite, Transform, BoxCollider, Rigidbody
class PlayerController : public System
{
public:
	PlayerController() {}

	void Update(GLFWwindow* window, double deltaTime, shared_ptr <PhysicsSystem> physicsSystem)
	{
		for (auto const& entity : entities)
		{
			Player& player = ecs.getComponent<Player>(entity);
			Rigidbody& rigidbody = ecs.getComponent<Rigidbody>(entity);
			BoxCollider& collider = ecs.getComponent<BoxCollider>(entity);

			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				physicsSystem->Move(entity, Vector2(player.moveSpeed, 0) * deltaTime);
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				physicsSystem->Move(entity, Vector2(-player.moveSpeed, 0) * deltaTime);
			}
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				physicsSystem->Move(entity, Vector2(0, player.moveSpeed) * deltaTime);
			}
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				physicsSystem->Move(entity, Vector2(0, -player.moveSpeed) * deltaTime);
			}


		}
	}
};