#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include "ECSCore.h"
#include "Transform.h"

using namespace std;

extern ECS ecs;

namespace engine
{
	//Sprite component
	struct Sprite
	{
		sf::Texture texture;
		int zOverride = 0;
		sf::Sprite sprite;
	};

	//Container for a single animation (not a component)
	struct Animation
	{
		vector<sf::Texture> textures;
		vector<int> delays;
		uint64_t length = 0;
	};

	//Animator Component
	struct Animator
	{
		map<string, Animation> animations;

		string currentAnimation;
		int animationFrame = 0;
		bool repeatAnimation = false;
		bool playingAnimation = false;

		sf::Clock animationTimer;
	};

	//Render System
	class RenderSystem :public System
	{
	public:
		void Update(sf::RenderWindow& window)
		{
			//Goes through list of entities, sets textures to sprites and draws them to window
			for (auto const& entity : entities)
			{
				//Get relevant components
				Sprite& sprite = ecs.getComponent<Sprite>(entity);
				Transform& transform = ecs.getComponent<Transform>(entity);

				//Set the sprite's texture, position, and scale
				sprite.sprite.setTexture(sprite.texture);
				sprite.sprite.setPosition(sf::Vector2f(transform.x - sprite.texture.getSize().x * transform.xScale / 2, transform.y - sprite.texture.getSize().y * transform.yScale / 2));
				sprite.sprite.setScale(sf::Vector2(transform.xScale, transform.yScale));
					
				//Draw the sprite to provided window
				window.draw(sprite.sprite);
			}
		}

	};

	//Animator system
	class AnimationSystem : public System
	{
	public:
		//Update every entity with relevant components
		void Update()
		{
			//For each entity that has the required components
			for (auto const& entity : entities)
			{
				//Get the relevant components from entity
				Animator& animator = ecs.getComponent<Animator>(entity);

				//If the entity is currently playing an animation
				if (animator.playingAnimation)
				{
					//If enough time (defined by animation) has passed
					if (animator.animationTimer.getElapsedTime().asMilliseconds() >= animator.animations[animator.currentAnimation].delays[animator.animationFrame])
					{
						AdvanceFrame(entity);
					}
				}
			}
		}

		//Advance to the next animation frame of current animation
		void AdvanceFrame(Entity entity)
		{
			//Get the relevant components from entity
			Animator& animator = ecs.getComponent<Animator>(entity);
			Sprite& sprite = ecs.getComponent<Sprite>(entity);

			//Change GameObject texture
			sprite.texture = animator.animations[animator.currentAnimation].textures[animator.animationFrame];

			animator.animationTimer.restart();
			animator.animationFrame++;

			//If end of animation has been reached go to start or end animation
			if (animator.animationFrame >= animator.animations[animator.currentAnimation].length)
			{
				animator.animationFrame = 0;
				if (!animator.repeatAnimation)
				{
					animator.playingAnimation = false;
					animator.currentAnimation = "";
				}
			}
		}

		//Add animations to entity, they will be accessible by given names
		void AddAnimations(Entity entity, vector<Animation> animations, vector<string> names)
		{
			if (animations.size() > names.size())
				throw("Not enough names for each animation!");

			Animator& animator = ecs.getComponent<Animator>(entity);

			//For each animation to add
			for (size_t i = 0; i < animations.size(); i++)
			{
				animator.animations.insert({ names[i], animations[i] });
			}
		}

		//Add an animation to entity, it will be accessibl by given name
		void AddAnimation(Entity entity, Animation animation, string name)
		{
			Animator& animator = ecs.getComponent<Animator>(entity);

			//Add the animation indexed by given name or number order
			animator.animations.insert({ name, animation });

		}

		//Play an animation, optionally set it to repeat
		void PlayAnimation(Entity entity, string animation, bool repeat = false)
		{
			Animator& animator = ecs.getComponent<Animator>(entity);

			animator.currentAnimation = animation;
			animator.animationFrame = 0;
			animator.repeatAnimation = repeat;
			animator.playingAnimation = true;
			animator.animationTimer.restart();
		}

		//Stop an animation, optionally provide the specific animation to stop
		void StopAnimation(Entity entity, string animation = "")
		{
			Animator& animator = ecs.getComponent<Animator>(entity);

			//If trying to stop animation that is not playing, return without doing anything
			if (animation != "")
				if (animator.currentAnimation != animation)
					return;

			animator.currentAnimation = "";
			animator.animationFrame = 0;
			animator.playingAnimation = false;
		}
	};

	//Slices an image into equal sized textures of width and height in pixels.
	//Returns a vector of textures ordered left to right top to bottom 
	vector<sf::Texture> SliceSpritesheet(sf::Image spritesheet, int width, int height)
	{
		//Throw warning if spritesheet is not properly dimensioned
		if (spritesheet.getSize().x % width != 0)
			cout << "Warning: Spritesheet width is not a multiple of sprite width. Clipping may occur!\n";
		if (spritesheet.getSize().y % height != 0)
			cout << "Warning: Spritesheet height is not a multiple of sprite height. Clipping may occur!\n";

		vector<sf::Texture> slicedSpritesheet;
		//Run through each full sprite in the spritesheet
		//if the final row or column are not full, skip them
		for (size_t y = 0; y < spritesheet.getSize().y - height + 1; y += height)
		{
			for (size_t x = 0; x < spritesheet.getSize().x - width + 1; x += width)
			{
				//Copy sector of spritesheet to new texture
				sf::Image slice;
				slice.create(width, height);
				slice.copy(spritesheet, 0, 0, sf::IntRect(x, y, width, height));

				sf::Texture slicedTexture;
				slicedTexture.loadFromImage(slice);

				slicedSpritesheet.push_back(slicedTexture);
			}
		}

		return slicedSpritesheet;
	}

	//Gets a subregion of an image by pixel coordinates from top-left to bottom-right
	//Return sliced texture
	sf::Texture CustomSlice(sf::Image spritesheet, int x1, int y1, int x2, int y2)
	{
		sf::Image slice;
		slice.create(x2 - x1, y2 - y1);
		slice.copy(spritesheet, 0, 0, sf::IntRect(x1, y1, x2, y2));

		sf::Texture slicedTexture = sf::Texture();
		slicedTexture.loadFromImage(slice);

		return slicedTexture;
	}

	//Automatically slice and create animations from a spritesheet with delays in ms and optional names.
	//Adds one animation per row of sprites in the spritesheet. You must provide one delay per sprite in the delays vector. 
	//All sprites must have the same width and height.
	vector<Animation> AnimationsFromSpritesheet(sf::Image spritesheet, int width, int height, vector<int> delays)
	{
		//Get a list of textures from the spritesheet
		vector<sf::Texture> textures = SliceSpritesheet(spritesheet, width, height);

		if (delays.size() < textures.size())
			throw("Not enough delays for amount of frames!\nYou must provide one delay after each frame (even the last one).\n");

		vector<Animation> newAnimations;

		cout << floor(spritesheet.getSize().y / height);

		//For each row in the spritesheet
		for (size_t y = 0; y < floor(spritesheet.getSize().y / height); y++)
		{
			//Frame buffer to add to animation
			vector<sf::Texture> animationSlice;
			vector<int> delaySlice;

			//For each column in the row
			for (size_t x = 0; x < floor(spritesheet.getSize().x / width); x++)
			{
				int a = floor(spritesheet.getSize().x / width) * y + x;
				//Add the next texture to the buffer 
				animationSlice.push_back(textures[floor(spritesheet.getSize().x / width) * y + x]);
				delaySlice.push_back(delays[floor(spritesheet.getSize().x / width)]);
			}

			Animation newAnimation{
				.textures = animationSlice,
				.delays = delaySlice,
				.length = animationSlice.size()
			};

			newAnimations.push_back(newAnimation);
		}
		return newAnimations;
	}

	//Create one animation from frames and delays in ms
	Animation CreateAnimation(vector<sf::Texture> frames, vector<int> delays) 
	{
		if (frames.size() != delays.size())
			throw("Not enough delays for amount of frames!\nYou must provide one delay after each frame (even the last one).\n");

		Animation newAnimation{
			.textures = frames,
			.delays = delays,
			.length = frames.size()
		};
		return newAnimation;
	}
}