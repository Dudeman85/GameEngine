#pragma once
#include "ECSCore.h"
#include <box2d/box2d.h>

extern ECS ecs;

namespace engine
{


	struct Rigidbody
	{
		b2Body* body;
	};

	// Gravity system
	class PhysicsSystem : public System
	{
	public:

		PhysicsSystem()
		{
			world = new b2World(gravity);
		}

		void Update()
		{
			//defines the timestep of the physics engine. Velocity iteration is 8 and position iteration 3. These are suggested by Box2d, smaller values decrease accuracy but increase performance
			world->Step(timeStep, 8, 3);

			for (auto const& entity : entities)
			{
				Rigidbody& rigidbody = ecs.getComponent<Rigidbody>(entity);
				Transform& transform = ecs.getComponent<Transform>(entity);
				transform.x = rigidbody.body->GetPosition().x;
				transform.y = rigidbody.body->GetPosition().y;
			}
		}

		void Init(float x, float y)
		{
			b2Vec2 gravity;
			gravity.Set(x, y);
			world->SetGravity(gravity);
		}


		//Creates new box2d rigidbody and gives size, density and friction to entity and defines whether or not it is static
		void DefineBody(Entity entity, float width, float height, float density = 1.f, float friction = 0.7f, bool isStatic = false)
		{
			Rigidbody& rigidbody = ecs.getComponent<Rigidbody>(entity);
			Transform& transform = ecs.getComponent<Transform>(entity);

			//defines a new body, it's position and whether or not it is static.
			b2BodyDef bodyDef;
			bodyDef.position = b2Vec2(transform.x / SCALE, transform.y / SCALE);
			bodyDef.type = isStatic ? b2_staticBody : b2_dynamicBody;

			//defines the size of created body
			b2PolygonShape shape;
			shape.SetAsBox((width / 2) / SCALE, (height / 2) / SCALE);

			//gives body its density/weight and friction
			b2FixtureDef fixtureDef;
			fixtureDef.density = density;
			fixtureDef.friction = friction;
			fixtureDef.shape = &shape;

			//sets entitys rigidbody
			rigidbody.body = world->CreateBody(&bodyDef);
			rigidbody.body->CreateFixture(&fixtureDef);
		}

	private:
		b2Vec2 gravity;
		bool fixedRotation;
		b2World* world;
		float timeStep = 1.0f / 60.0f;
		const float SCALE = 64.f;

		//PhysicsSystem();
		PhysicsSystem(const PhysicsSystem&);
		PhysicsSystem& operator=(const PhysicsSystem&);

	};


}
