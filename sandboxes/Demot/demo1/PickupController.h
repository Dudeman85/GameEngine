#pragma once
#include <engine/Application.h>
#include <cmath>
#include <algorithm>

extern ECS ecs;

using namespace engine;

struct Pickup
{
	bool floating = true;
	int sound = 0;
};

//Pickup controller system requires sprite, transform, rigidbody, box collider, Animator and Pickup
class PickupController : public System
{
public:
	PickupController()
	{
		defaultTexture = new Texture("assets/strawberry.png");
		winner = new Texture("assets/winner.png");
		animations = AnimationsFromSpritesheet("assets/Strawberry Animation.png", 7, 2, vector<int>(7*2, 100));

		winScreen = ecs.newEntity();
		ecs.addComponent(winScreen, Transform{.z = 20, .xScale = 200, .yScale = 200});
		ecs.addComponent(winScreen, Sprite{.texture = winner, .enabled = false});
	}

	void Update(Entity player, double programTime)
	{
		for (auto const& entity : entities)
		{
			Pickup& pickup = ecs.getComponent<Pickup>(entity);
			Transform& transform = ecs.getComponent<Transform>(entity);
			BoxCollider& collider = ecs.getComponent<BoxCollider>(entity);
			Animator& animator = ecs.getComponent<Animator>(entity);

			//If floating animate it
			if (pickup.floating)
				transform.y += sin(programTime * 3) * 0.5;

			//If collided with the player start the collection animation
			if (collider.collisions.end() != find_if(collider.collisions.begin(), collider.collisions.end(), [player](const Collision& collision) { return collision.a == player; }))
			{
				AnimationSystem::PlayAnimation(entity, "collect");
				pickup.sound = 1;
			}

			//When the collection animation is over
			if (!animator.playingAnimation)
			{
				collected++;
				ecs.destroyEntity(entity);
				break;
			}
		}

		if (collected >= total)
		{
			ecs.getComponent<Sprite>(winScreen).enabled = true;
		}
		TransformSystem::SetPosition(winScreen, Vector3(ecs.getComponent<Transform>(player).x, ecs.getComponent<Transform>(player).y, 20));
	}

	Entity CreatePickup(float x, float y)
	{
		Entity pickup = ecs.newEntity();
		ecs.addComponent(pickup, Transform{ .x = x, .y = y, .z = 5, .xScale = 30, .yScale = 25 });
		ecs.addComponent(pickup, Sprite{ .texture = defaultTexture });
		ecs.addComponent(pickup, Rigidbody{ .kinematic = true });
		ecs.addComponent(pickup, BoxCollider{ .isTrigger = true });
		ecs.addComponent(pickup, Pickup{});
		ecs.addComponent(pickup, Animator{});
		AnimationSystem::AddAnimations(pickup, animations, vector<string>{"default", "collect"});
		AnimationSystem::PlayAnimation(pickup, "default", true);
		total++;

		return pickup;
	}

	Entity CreateScoreboard(float x, float y)
	{
		Entity board = ecs.newEntity();
		ecs.addComponent(board, Transform{ .x = x, .y = y });
		ecs.addComponent(board, Animator{});
	}

	Entity winScreen;
	Texture* winner;
	vector<Animation> animations;
	Texture* defaultTexture;
	int collected = 0;
	int total = 0;
};