#pragma once

// TODO: Need to see how to use multiple Text TTF files.

// FreeType includes
#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <engine/GL/Shader.h>
#include <engine/GL/Camera.h>
#include <map>
#include <vector>

using namespace std;

struct Character
{
	unsigned int TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	unsigned int Advance;
};

struct TrueFont
{
	int vecOrder;
	const char* filepathname;
	FT_Long face_index;
	FT_UInt pixel_width;
	FT_UInt pixel_height;
	FT_Face aface;
};

namespace engine
{
	class TextRender
	{
	public:
		// Constructor
		TextRender();
		
		// Destructor
		~TextRender();

		// Multiple TrueFonts function
		TrueFont SetUpTTF(const char* filepathname, FT_Long face_index, FT_UInt pixel_width, FT_UInt pixel_height);


		// VAO & VBO function
		void TexConfig();

		// Text loading function
		void LoadText(TrueFont truefont);
		
		// Text Rendering function
		void RenderText(TrueFont trueFont, Camera* cam, string text, float x, float y, float scale, glm::vec3 colour);

	private:
		map<GLchar, Character> Characters;

		TrueFont TrueFonts;

		unsigned int VAO, VBO;

		unsigned int texture;
		
		int TTForder;

		FT_Library ft;
		FT_Face face;

		glm::mat4 projection;
		glm::mat4 viewMatrix;

		engine::Shader* m_shader;
	};
}