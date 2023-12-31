#pragma once
//OpenGL
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//STL
#include <vector>
#include <set>

//Engine
#include <engine/ECSCore.h>
#include <engine/Transform.h>
#include <engine/GL/Shader.h>
#include <engine/GL/Texture.h>
#include <engine/GL/Camera.h>
#include <engine/Tilemap.h>

extern ECS ecs;

using namespace std;

namespace engine
{
	//2D Sprite Renderer component
	struct SpriteRenderer
	{
		Texture* texture = nullptr;
		Shader* shader = nullptr;
		bool enabled = true;
		bool uiElement = false;
	};

	//Animation struct. Not a component
	struct Animation
	{
		Animation() {};
		Animation(vector<Texture*> animationTextures, vector<int> animationDelays)
		{
			assert(animationTextures.size() == animationDelays.size() && "Failed to create animation! Number of frames and delays do not match!");
			textures = animationTextures;
			delays = animationDelays;
			length = animationDelays.size();
		};
		vector<Texture*> textures;
		vector<int> delays;
		unsigned int length = 0;
	};

	//Animator component
	struct Animator
	{
		map<string, Animation> animations;

		string currentAnimation;
		int animationFrame = 0;
		bool repeatAnimation = false;
		bool playingAnimation = false;

		float animationTimer = 0;
	};

	//2D Sprite Render system
	//Requires SpriteRenderer and Transform
	class SpriteRenderSystem : public System
	{
	public:
		SpriteRenderSystem()
		{
			//Set the screen clear color to black
			glClearColor(0, 0, 0, 1.0f);

			//Enable transparency
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			//Enable Depth buffering
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			//Create the default sprite shader
			defaultShader = new Shader(
				R"(
				#version 330 core
				layout (location = 0) in vec3 aPos;
				layout(location = 1) in vec2 aTexCoord;
				out vec2 TexCoord;
				uniform mat4 model;
				uniform mat4 view;
				uniform mat4 projection;
				void main()
				{
					gl_Position = projection * view * model * vec4(aPos, 1.0f);
					TexCoord = vec2(aTexCoord.x, aTexCoord.y);
				}
				)",
				R"(
				#version 330 core
				out vec4 FragColor;
				in vec2 TexCoord;
				uniform sampler2D texture1;
				void main()
				{
				   FragColor = texture(texture1, TexCoord);
				}
				)", false);

			//Rectangle vertices start at top left and go clockwise to bottom left
			float vertices[] = {
				//Positions		  Texture Coords
				 1.f,  1.f, 0.0f, 1.0f, 1.0f, // top right
				 1.f, -1.f, 0.0f, 1.0f, 0.0f, // bottom right
				-1.f, -1.f, 0.0f, 0.0f, 0.0f, // bottom left
				-1.f,  1.f, 0.0f, 0.0f, 1.0f, // top left 
			};
			//Indices to draw a rectangle from two triangles
			unsigned int indices[] = {
				0, 1, 2, //1st trangle
				0, 2, 3, //2nd triangle
			};

			//Make the Vertex Array Object, Vertex Buffer Object, and Element Buffer Object
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			//Bind the Vertex Array Object
			glBindVertexArray(VAO);

			//Bind the Vertex Bufer Object and set vertices
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			//Bind and set indices to EBO
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

			//Configure Vertex attribute at location 0 aka position
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			//Configure Vertex attribute at location 1 aka texture coords
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			//Unbind all buffers and arrays
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		~SpriteRenderSystem()
		{
			glDeleteVertexArrays(1, &VAO);
			glDeleteVertexArrays(1, &VBO);
			glDeleteVertexArrays(1, &EBO);
		}

		//Renders everything. Call this every frame
		void Update(Camera* cam)
		{
			//Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//Sort the entities and tilemap by Z
			set<float> layersToDraw;
			if (tilemap)
				layersToDraw.insert(tilemap->zLayers.begin(), tilemap->zLayers.end());
			map<float, vector<Entity>> sortedEntities;
			for (const Entity& entity : entities)
			{
				Transform& transform = ecs.getComponent<Transform>(entity);
				sortedEntities[transform.position.z].push_back(entity);
				layersToDraw.insert(transform.position.z);
			}

			//Draw everything by layer
			for (const float& layer : layersToDraw)
			{
				if (tilemap)
					tilemap->draw(layer);

				//Bind the right VAO after tilemap
				glBindVertexArray(VAO);

				//Draw entities for this layer
				for (const Entity& entity : sortedEntities[layer])
				{
					//Get relevant components
					SpriteRenderer& sprite = ecs.getComponent<SpriteRenderer>(entity);
					Transform& transform = ecs.getComponent<Transform>(entity);

					if (!sprite.enabled)
						continue;

					//If a shader has been specified for this sprite use it, else use the default
					Shader* shader = defaultShader;
					if (sprite.shader)
						shader = sprite.shader;
					shader->use();

					//Create the model matrix
					glm::mat4 model = glm::mat4(1.0f);
					//Position
					model = glm::translate(model, transform.position.ToGlm());
					//X, Y, Z euler rotations
					model = glm::rotate(model, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
					model = glm::rotate(model, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
					model = glm::rotate(model, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
					//Scale
					model = glm::scale(model, transform.scale.ToGlm());

					//Give the shader the model matrix
					unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");
					glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

					//Get the view and projection locations
					unsigned int viewLoc = glGetUniformLocation(shader->ID, "view");
					unsigned int projLoc = glGetUniformLocation(shader->ID, "projection");

					if (!sprite.uiElement)
					{
						//Give the shader the camera's view matrix
						glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam->GetViewMatrix()));

						//Give the shader the camera's projection matrix
						glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam->GetProjectionMatrix()));
					}
					else
					{
						//Give the shader a constant view matrix
						glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

						//Give the shader a constant projection matrix
						glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
					}

					//Bind the texture
					glActiveTexture(GL_TEXTURE0);
					if (sprite.texture)
						sprite.texture->Use();

					//Draw the sprite
					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

					//Unbind the texture
					glBindTexture(GL_TEXTURE_2D, 0);
				}
			}

			//Unbind vertex array
			glBindVertexArray(0);
		}

		//Set the screens clear color to given rgb
		static void SetBackgroundColor(float r, float g, float b)
		{
			glClearColor(r / 255, g / 255, b / 255, 1.0f);
		}

		//Set a tilmap to render
		void SetTilemap(Tilemap* map)
		{
			tilemap = map;
		}

	private:
		unsigned int VAO, VBO, EBO;
		Shader* defaultShader;
		Tilemap* tilemap = nullptr;
	};

	//Animator system
	//Requires Animator and Sprite
	class AnimationSystem : public System
	{
	public:
		//Update every entity with relevant components
		void Update(float deltaTime)
		{
			//Delta time in milliseconds
			deltaTime *= 1000;

			//For each entity that has the required components
			for (auto const& entity : entities)
			{
				//Get the relevant components from entity
				Animator& animator = ecs.getComponent<Animator>(entity);

				//If the entity is currently playing an animation
				if (animator.playingAnimation)
				{
					animator.animationTimer += deltaTime;

					//If enough time (defined by animation) has passed advance the animation frame
					if (animator.animationTimer >= animator.animations[animator.currentAnimation].delays[animator.animationFrame])
					{
						AdvanceFrame(entity);
					}
				}
			}
		}

		//Advance to the next animation frame of current animation
		static void AdvanceFrame(Entity entity)
		{
			//Get the relevant components from entity
			Animator& animator = ecs.getComponent<Animator>(entity);
			SpriteRenderer& sprite = ecs.getComponent<SpriteRenderer>(entity);

			//Change Sprites texture
			sprite.texture = animator.animations[animator.currentAnimation].textures[animator.animationFrame];

			animator.animationFrame++;
			animator.animationTimer = 0;

			//If end of animation has been reached go to start or end animation
			if (animator.animationFrame >= animator.animations[animator.currentAnimation].length)
			{
				animator.animationFrame = 0;
				animator.animationTimer = 0;

				//End the animation if it is not set to repeat
				if (!animator.repeatAnimation)
				{
					animator.playingAnimation = false;
					animator.currentAnimation = "";
					return;
				}
			}
		}

		//Add animations to entity, they will be accessible by given names
		static void AddAnimations(Entity entity, vector<Animation> animations, vector<string> names)
		{
			if (animations.size() > names.size())
				throw("Not enough names given for each animation!");

			Animator& animator = ecs.getComponent<Animator>(entity);

			//For each animation to add
			for (size_t i = 0; i < animations.size(); i++)
			{
				animator.animations.insert({ names[i], animations[i] });
			}
		}

		//Add an animation to entity, it will be accessibl by given name
		static void AddAnimation(Entity entity, Animation animation, string name)
		{
			Animator& animator = ecs.getComponent<Animator>(entity);

			//Add the animation indexed by given name
			animator.animations.insert({ name, animation });
		}

		//Play an animation, optionally set it to repeat, if the animation is currently playing don't do anything
		static void PlayAnimation(Entity entity, string animation, bool repeat = false)
		{
			Animator& animator = ecs.getComponent<Animator>(entity);

			if (animator.animations.find(animation) == animator.animations.end())
			{
				cout << "Warning: No animation named \"" << animation << "\" was found in this entity." << endl;
				return;
			}

			if (animator.currentAnimation == animation)
				return;

			animator.currentAnimation = animation;
			animator.animationFrame = 0;
			animator.repeatAnimation = repeat;
			animator.playingAnimation = true;
			animator.animationTimer = 0;

			//Immediately advance to 1st frame of animation
			AdvanceFrame(entity);
		}

		//Stop an animation, optionally provide the specific animation to stop
		static void StopAnimation(Entity entity, string animation = "")
		{
			Animator& animator = ecs.getComponent<Animator>(entity);

			//If trying to stop animation that is not playing, return without doing anything
			if (animation != "")
				if (animator.currentAnimation != animation)
					return;

			animator.currentAnimation = "";
			animator.animationFrame = 0;
			animator.animationTimer = 0;
			animator.playingAnimation = false;
		}
	};
}