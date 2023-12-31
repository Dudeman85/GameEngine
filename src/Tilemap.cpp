/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

tmxlite - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/
#define _USE_MATH_DEFINES
#include <engine/Tilemap.h>
#include <glm/gtc/matrix_transform.hpp>
#include <tmxlite/Map.hpp>
#include <engine/GL/Shader.h>
#include <engine/GL/Texture.h>
#include <engine/MapLayer.h>
#include <cassert>
#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <tmxlite/TileLayer.hpp>
#include <algorithm>

Tilemap::Tilemap(engine::Camera* cam)
{
	m_shader = new engine::Shader("tilemapVertexShader.glsl", "tilemapFragmentShader.glsl");
	camera = cam;
	collisionLayer = std::vector<std::vector<unsigned int>>();
	position = glm::vec3(0);
}

Tilemap::~Tilemap()
{
}

void Tilemap::draw(float layer)
{
	if (mapLayers.count(layer) == 0)
		return;

	m_shader->use();

	glm::mat4 model = glm::translate(glm::mat4(1.0f), position);

	unsigned int modelLoc = glGetUniformLocation(m_shader->ID, "u_modelMatrix");

	unsigned int viewLoc = glGetUniformLocation(m_shader->ID, "u_viewMatrix");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

	//Give the shader the projection matrix
	unsigned int projLoc = glGetUniformLocation(m_shader->ID, "u_projectionMatrix");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

	unsigned int u_tilesetCount = glGetUniformLocation(m_shader->ID, "u_tilesetCount");

	unsigned int u_tileSize = glGetUniformLocation(m_shader->ID, "u_tileSize");

	for (int i = 0; i < mapLayers[layer].size(); i++)
	{
		mapLayers[layer][i]->draw(model, modelLoc, u_tilesetCount, u_tileSize);
	}
}

void Tilemap::loadMap(const std::string ownMap)
{
	tmx::Map map;
	map.load(ownMap);

	//create shared resources, shader and tileset textures
	initGLStuff(map);

	bounds = map.getBounds();
	tileSize = map.getTileSize();
	zLayers.insert(0);

	//create a drawable object for each tile layer
	const auto& layers = map.getLayers();
	for (auto i = 0u; i < layers.size(); ++i)
	{
		if (layers[i]->getType() == tmx::Layer::Type::Tile)
		{
			//If the layer is a collision layer
			if (layers[i]->getName() == "collider")
			{
				//Resize the collision map
				collisionLayer.resize(map.getTileCount().x);
				for (int i = 0; i < collisionLayer.size(); i++)
				{
					collisionLayer[i].resize(map.getTileCount().y);
				}

				//Get the tile IDs
				auto& tiles = layers[i]->getLayerAs<tmx::TileLayer>().getTiles();

				//Transfer the tile IDs to the 2D collision vector
				for (int y = 0; y < collisionLayer[0].size(); y++)
				{
					for (int x = 0; x < collisionLayer.size(); x++)
					{
						collisionLayer[x][y] = tiles[(y * collisionLayer.size()) + x].ID;
					}
				}
			}
			else
			{
				std::shared_ptr<MapLayer> layer = std::make_unique<MapLayer>(map, i, allTextures);

				const auto& properties = layers[i]->getProperties();
				tmx::Tileset tileset = map.getTilesets()[0];
				layer->tileSize = glm::vec2(tileset.getTileSize().x, tileset.getTileSize().y);
				for (const auto& property : properties)
				{
					if (property.getName() == "Z")
					{
						zLayers.insert(property.getFloatValue());
						layer->zLayer = property.getFloatValue();
						break;
					}
				}

				mapLayers[layer->zLayer].push_back(layer);
			}
		}
	}
}

//Returns the collision layers tile ID at x and y
unsigned int Tilemap::checkCollision(float x, float y)
{
	if (collisionLayer.empty())
		return 0;

	int xIndex = floor((x + position.x) / tileSize.x);
	int yIndex = floor((-y + position.y) / tileSize.y);

	//Check out of bounds
	if (xIndex >= collisionLayer.size() || yIndex >= collisionLayer[0].size() || xIndex < 0 || yIndex < 0)
		return 0;

	return collisionLayer[xIndex][yIndex];
}

void Tilemap::initGLStuff(const tmx::Map& map)
{
	m_shader->use();

	//we'll make sure the current tile texture is active in 0, 
	//and lookup texture is active in 1 in MapLayer::draw()
	glUniform1i(glGetUniformLocation(m_shader->ID, "u_tileMap"), 0);
	glUniform1i(glGetUniformLocation(m_shader->ID, "u_lookupMap"), 1);


	const auto& tilesets = map.getTilesets();
	for (const auto& ts : tilesets)
	{
		auto texture = loadTexture(ts.getImagePath());
		allTextures.push_back(texture);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
}

std::shared_ptr<engine::Texture> Tilemap::loadTexture(const std::string& path)
{
	return std::make_shared<engine::Texture>(path.c_str(), GL_NEAREST, false);
}