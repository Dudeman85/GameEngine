#pragma once
//Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//STL 
#include <string>
#include <vector>

//Engine
#include <engine/ECSCore.h>
#include <engine/Transform.h>
#include <engine/GL/Shader.h>
#include <engine/GL/Texture.h>
#include <engine/GL/Camera.h>
#include <engine/GL/Mesh.h>

extern ECS ecs;

namespace engine
{
	//A class to store 3D model vertex and texture data, as well as handle model loading
	class Model
	{
	public:
		Model(const char* path)
		{
			LoadModel(path);
		}
		~Model()
		{
			for (Texture* tex : textures_loaded)
			{
				delete tex;
			}
		}

		//Model data
		std::vector<Mesh> meshes;

	private:
		void LoadModel(std::string path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

		//Model file path
		std::string directory;
		//Store all the already loaded textures for efficiency
		std::vector<Texture*> textures_loaded;
	};

	//3D Model Renderer component
	struct ModelRenderer
	{
		Model* model;
		Shader* shader;
	};

	//3D Model Render System, requires Transform and ModelRenderer
	class ModelRenderSystem : public System
	{
	public:
		ModelRenderSystem()
		{
			//The default 3D model shader
			defaultShader = new Shader(
				R"( //////////////// vexterShader ////////////////////////////
				#version 330 core
				layout(location = 0) in vec3 aPos;
				layout(location = 1) in vec3 aNormal;
				layout(location = 2) in vec2 aTexCoords;
                
				
                out vec3 FragPos;
                out vec3 Normal;
                out vec2 TexCoords;


				uniform mat4 model;
				uniform mat4 view;
				uniform mat4 projection;

				void main()
				{
					TexCoords = aTexCoords;
                    FragPos = vec3 (model * vec4(aPos,1.0));
                    Normal = mat3(transpose(inverse(model))) *aNormal;
 
					gl_Position = projection * view * model * vec4(aPos, 1.0);
                    
				}
				)",
				R"( ////////////////// FragmerSheder///////////////////////////////////////////////
				#version 330 core
				out vec4 FragColor;
 

				in vec2 TexCoords;
                in vec3 Normal;
                in vec3 FragPos;

                uniform sampler2D texture_diffuse1;
				uniform vec3 lightPos;
                uniform vec3 lightColor;
                uniform vec3 viewPos;
 
				void main()
				{    
					vec3 objectColor = texture(texture_diffuse1, TexCoords).xyz;
//////////////////////////////////// Ambient //////////////////////////////////////////////////////////
					float ambientStrength = 0.3;
					vec3 ambient = ambientStrength * lightColor;
                    
///////////////////////////////// diffuce //////////////////////////////////////////////////////////////
                    vec3 norm = normalize(Normal);
                    vec3 lightDir = normalize(lightPos - FragPos);
                    float diff = max(dot(norm , lightDir), 0.0);
                    vec3 diffuse = diff * lightColor;

/////////////////////////////// Specular //////////////////////////////////////////////////////////////////
                    float specularStrength = 0.5;
                    vec3 viewDir = normalize(viewPos - FragPos);
                    vec3 reflectDir = reflect(-lightDir, norm);  
                    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
                    vec3 specular = specularStrength * spec * lightColor; 

//////////////////////////////// Phong ///////////////////////////////////////////////////////////////////// 
                   vec3 result = (ambient + diffuse + specular ) * objectColor;
                   FragColor = vec4(result, 1.0);  
				 
				}
				)", false);
			
		}

		void Update(Camera* cam)
		{
			//For each entity
			for (const Entity& entity : entities)
			{
				//Get relevant components
				Transform& transform = ecs.getComponent<Transform>(entity);
				ModelRenderer& modelRenderer = ecs.getComponent<ModelRenderer>(entity);

				//If a shader has been specified for this sprite use it, else use the default
				Shader* shader = defaultShader;
				if (modelRenderer.shader)
					shader = modelRenderer.shader;
				shader->use();

				//Create the model matrix, this is the same for each mesh so it only needs to be done once
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

				//Give the shader the view matrix
				unsigned int viewLoc = glGetUniformLocation(shader->ID, "view");
				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cam->GetViewMatrix()));

				//Give the shader the projection matrix
				unsigned int projLoc = glGetUniformLocation(shader->ID, "projection");
				glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(cam->GetProjectionMatrix()));

                 // lighting color
				unsigned int colorLoc = glGetUniformLocation(shader->ID, "lightColor");
				glUniform3f(colorLoc, lightColor.x/255, lightColor.y/255 , lightColor.z/255);

				// lighting position
				unsigned int lightPosLoc = glGetUniformLocation(shader->ID, "lightPos");
				glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);

				// camara pposition
				unsigned int viewPosLoc = glGetUniformLocation(shader->ID, "viewPos");
				glUniform3fv(viewPosLoc,1, glm::value_ptr(cam->position));


				//For each mesh in the model
				for (unsigned int i = 0; i < modelRenderer.model->meshes.size(); i++)
				{
					Mesh mesh = modelRenderer.model->meshes[i];

					//Texture uniforms are named: uniform sampler2D texture_diffuseN, or texture_specularN
					//We can support up to 8 textures which have to be defined in the shader
					unsigned int diffuseNr = 1;
					unsigned int specularNr = 1;

					//For each Texture in the mesh
					for (unsigned int i = 0; i < mesh.textures.size(); i++)
					{
						//Activate proper texture unit before binding
						glActiveTexture(GL_TEXTURE0 + i);

						//Retrieve texture number and type (the N in texture_{type}N)
						std::string number;
						std::string name = mesh.textures[i]->type;
						if (name == "texture_diffuse")
							number = std::to_string(diffuseNr++);
						else if (name == "texture_specular")
							number = std::to_string(specularNr++);

						//Set the uniform for the material texture
						glUniform1i(glGetUniformLocation(shader->ID, (/*"material." + */name + number).c_str()), i);

						glBindTexture(GL_TEXTURE_2D, mesh.textures[i]->ID());
					}

					//Unbind texture
					glActiveTexture(GL_TEXTURE0);

					//Draw mesh
					glBindVertexArray(mesh.VAO);
					glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
				}
			}
		}

		Vector3 lightPos;
		Vector3 lightColor = Vector3(255);


		Shader* defaultShader;
	};
}