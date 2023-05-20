#include <engine/Application.h>
#include <engine/Tilemap.h>

#include "PlayerController.h"
#include "PickupController.h"
#include "EnemyController.h"
#include "TurretController.h"

#include <chrono>
#include <thread>

using namespace std;
using namespace engine;

ECS ecs;

int main()
{
	//Create the window and OpenGL context before creating EngineLib
	GLFWwindow* window = CreateWindow(1000, 800, "Window");

	//Create the camera
	Camera cam = Camera(1000, 800);

	//Initialize the default engine library
	EngineLib engine;

	//Register Custom Systems
	//Register Player Controller
	ecs.registerComponent<Player>();
	shared_ptr<PlayerController> playerController = ecs.registerSystem<PlayerController>();
	Signature playerControllerSignature;
	playerControllerSignature.set(ecs.getComponentId<Transform>());
	playerControllerSignature.set(ecs.getComponentId<Player>());
	playerControllerSignature.set(ecs.getComponentId<Sprite>());
	playerControllerSignature.set(ecs.getComponentId<Rigidbody>());
	playerControllerSignature.set(ecs.getComponentId<BoxCollider>());
	playerControllerSignature.set(ecs.getComponentId<Animator>());
	ecs.setSystemSignature<PlayerController>(playerControllerSignature);
	//Register Pickup Controller
	ecs.registerComponent<Pickup>();
	shared_ptr<PickupController> pickupController = ecs.registerSystem<PickupController>();
	Signature pickupControllerSignature;
	pickupControllerSignature.set(ecs.getComponentId<Transform>());
	pickupControllerSignature.set(ecs.getComponentId<Pickup>());
	pickupControllerSignature.set(ecs.getComponentId<Sprite>());
	pickupControllerSignature.set(ecs.getComponentId<Rigidbody>());
	pickupControllerSignature.set(ecs.getComponentId<BoxCollider>());
	ecs.setSystemSignature<PickupController>(pickupControllerSignature);
	//Register Enemy Controller
	ecs.registerComponent<Enemy>();
	shared_ptr<EnemyController> enemyController = ecs.registerSystem<EnemyController>();
	Signature enemyControllerSignature;
	enemyControllerSignature.set(ecs.getComponentId<Transform>());
	enemyControllerSignature.set(ecs.getComponentId<Enemy>());
	enemyControllerSignature.set(ecs.getComponentId<Sprite>());
	enemyControllerSignature.set(ecs.getComponentId<Rigidbody>());
	enemyControllerSignature.set(ecs.getComponentId<BoxCollider>());
	enemyControllerSignature.set(ecs.getComponentId<Animator>());
	ecs.setSystemSignature<EnemyController>(enemyControllerSignature);
	//Register Turret Controller
	ecs.registerComponent<Turret>();
	shared_ptr<TurretController> turretController = ecs.registerSystem<TurretController>();
	Signature turretControllerSignature;
	turretControllerSignature.set(ecs.getComponentId<Transform>());
	turretControllerSignature.set(ecs.getComponentId<Turret>());
	turretControllerSignature.set(ecs.getComponentId<Sprite>());
	turretControllerSignature.set(ecs.getComponentId<Rigidbody>());
	turretControllerSignature.set(ecs.getComponentId<BoxCollider>());
	turretControllerSignature.set(ecs.getComponentId<Animator>());
	ecs.setSystemSignature<TurretController>(turretControllerSignature);

	static SoundSource waterSpeaker;
	static SoundSource mageSpeaker;
	static SoundSource torchSpeaker;
	/*static SoundSource speaker;
	static SoundSource walkSpeaker;
	static SoundSource swordSpeaker;*/

	//Create the player entity
	Entity player = ecs.newEntity();
	Entity playerAttack = ecs.newEntity();
	Transform& playerTransform = ecs.addComponent(player, Transform{ .x = 110, .y = 200, .z = 0, .xScale = 50, .yScale = 50 });
	ecs.addComponent(player, Sprite{});
	ecs.addComponent(player, Animator{});
	ecs.addComponent(player, Rigidbody{});
	ecs.addComponent(player, BoxCollider{ .scale = Vector2(0.2, 0.61), .offset = Vector2(0, -18) });
	ecs.addComponent(player, Player{ .attackHitbox = playerAttack });

	//Create the player's attack hitbox
	ecs.addComponent(playerAttack, Sprite{});
	ecs.addComponent(playerAttack, Transform{ .xScale = 10, .yScale = 20 });
	ecs.addComponent(playerAttack, Rigidbody{ .kinematic = true });
	ecs.addComponent(playerAttack, BoxCollider{ .isTrigger = true });

	//Add animation to player
	vector<Animation> anims = AnimationsFromSpritesheet("assets/warriorsheet.png", 8, 4, vector<int>(8 * 4, 100));
	AnimationSystem::AddAnimations(player, anims, vector<string>{"Idle", "Run", "Wallslide", "Jump"});
	vector<Animation> attackAnims = AnimationsFromSpritesheet("assets/warriorattack.png", 4, 4, vector<int>(4 * 4, 100));
	AnimationSystem::AddAnimations(player, attackAnims, vector<string>{"Jump Attack", "Attack 1", "Attack 2", "Attack 3"});

	Entity waterfall = ecs.newEntity();
	ecs.addComponent(waterfall, Transform{ .x = 1200, .y = -1154, .z = -10, .xScale = 110, .yScale = 50 });
	ecs.addComponent(waterfall, Sprite{});
	ecs.addComponent(waterfall, Animator{});
	vector<Animation> waterfallAnims = AnimationsFromSpritesheet("assets/waterfall01.png", 4, 1, vector<int>(4 * 1, 100));
	AnimationSystem::AddAnimation(waterfall, waterfallAnims[0], "1");
	AnimationSystem::PlayAnimation(waterfall, "1", true);
	Entity waterfall2 = ecs.newEntity();
	ecs.addComponent(waterfall2, Transform{ .x = 1200, .y = -1251, .z = -15, .xScale = 110, .yScale = 50 });
	ecs.addComponent(waterfall2, Sprite{});
	ecs.addComponent(waterfall2, Animator{});
	vector<Animation> waterfallAnims2 = AnimationsFromSpritesheet("assets/waterfall01.png", 4, 1, vector<int>(4 * 1, 100));
	AnimationSystem::AddAnimation(waterfall2, waterfallAnims2[0], "1");
	AnimationSystem::PlayAnimation(waterfall2, "1", true);
	Entity waterfall3 = ecs.newEntity();
	ecs.addComponent(waterfall3, Transform{ .x = 1200, .y = -1348, .z = -15, .xScale = 110, .yScale = 50 });
	ecs.addComponent(waterfall3, Sprite{});
	ecs.addComponent(waterfall3, Animator{});
	vector<Animation> waterfallAnims3 = AnimationsFromSpritesheet("assets/waterfall01.png", 4, 1, vector<int>(4 * 1, 100));
	AnimationSystem::AddAnimation(waterfall3, waterfallAnims3[0], "1");
	AnimationSystem::PlayAnimation(waterfall3, "1", true);

	Entity torch = ecs.newEntity();
	ecs.addComponent(torch, Transform{ .x = 1480, .y = -2000, .z = -10, .xScale = 15, .yScale = 35 });
	ecs.addComponent(torch, Sprite{});
	ecs.addComponent(torch, Animator{});
	vector<Animation> torchAnims = AnimationsFromSpritesheet("assets/torch.png", 9, 1, vector<int>(9 * 1, 40));
	AnimationSystem::AddAnimation(torch, torchAnims[0], "1");
	AnimationSystem::PlayAnimation(torch, "1", true);

	RenderSystem::SetBackgroundColor(0, .5, .1);

	Tilemap map(&cam);
	map.loadMap("assets/level01.tmx");
	//Set the gravity and tilemap collider
	engine.physicsSystem->gravity = Vector2(0, -10000);
	engine.physicsSystem->SetTilemap(&map);
	engine.renderSystem->SetTilemap(&map);

	waterSpeaker.setLinearDistanceClamped(1, 1.0f, 150.f, 900.f, 1.f);
	mageSpeaker.setLinearDistanceClamped(2, 1.f, 100.f, 600.f, 1.f);
	torchSpeaker.setLinearDistanceClamped(3, 0.5f, 50.f, 500.f, 1.f);
	/*speaker.setLinearDistanceClamped(3, 1.f, 100.f, 600.f, 1.f);
	walkSpeaker.setLinearDistanceClamped(2, 1.f, 100.f, 600.f, 1.f);
	mageSpeaker.setLinearDistanceClamped(4, 1.f, 100.f, 600.f, 1.f);
	swordSpeaker.setLinearDistanceClamped(5, 1.f, 100.f, 600.f, 1.f);*/

	pickupController->CreatePickup(1780, -840);
	pickupController->CreatePickup(915, -420);
	pickupController->CreatePickup(624, -1250);
	pickupController->CreatePickup(1750, -1505);

	turretController->player = player;
	turretController->CreateTurret(500, 0);

	uint32_t swingSound = SoundBuffer::getFile()->addSoundEffect("assets/swing.wav");
	uint32_t waterSound = SoundBuffer::getFile()->addSoundEffect("assets/stream.wav");
	uint32_t torchSound = SoundBuffer::getFile()->addSoundEffect("assets/fire.wav");

	mageSpeaker.Play(swingSound);
	mageSpeaker.SetLooping(1);

	waterSpeaker.Play(waterSound);
	waterSpeaker.SetLooping(1);

	torchSpeaker.Play(torchSound);
	torchSpeaker.SetLooping(1);
	//Game Loop
	while (!glfwWindowShouldClose(window))
	{
		//std::this_thread::sleep_for(std::chrono::milliseconds{100});

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
		{
			cout << playerTransform.x << ", " << playerTransform.y << endl;
		}

		playerController->Update(window, engine.deltaTime /*,speaker, walkSpeaker, swordSpeaker*/);
		pickupController->Update(player, engine.programTime);
		turretController->Update(engine.deltaTime);

		//Update all engine systems, this usually should go last in the game loop
		//For greater control of system execution, you can update each one manually
		engine.Update(&cam);

		//Keep the camera in bounds of the tilemap
		float camPosX = clamp(playerTransform.x, map.position.x + cam.width / 2, map.position.x + map.bounds.width - cam.width / 2);
		float camPosY = clamp(playerTransform.y, map.position.y - map.bounds.height + cam.width / 2, map.position.y - cam.width / 2);
		cam.SetPosition(camPosX, camPosY, 100);


		engine.soundDevice->SetOrientation(0.f, 1.f, 0.f, 0.f, 0.f, 1.f);

		engine.soundDevice->SetLocation(playerTransform.x, playerTransform.y, 0);

		engine.soundDevice->SetSourceLocation(1, 1200, -1300, -35);
		engine.soundDevice->SetSourceLocation(2, 500, 0, 1);
		engine.soundDevice->SetSourceLocation(3, 1480, -300, 1);
		/*engine.soundDevice->SetSourceLocation(3, playerTransform.x, playerTransform.y, 0);
		engine.soundDevice->SetSourceLocation(2, playerTransform.x, playerTransform.y, 0);
		engine.soundDevice->SetSourceLocation(4, playerTransform.x, playerTransform.y, 1);
		engine.soundDevice->SetSourceLocation(4, 500, 0, 1);*/

		//OpenGL stuff, goes very last
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}