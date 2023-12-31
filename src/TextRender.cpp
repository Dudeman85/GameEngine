#include "engine/TextRender.h"

using namespace engine;

TextRender::TextRender()
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_shader = new Shader("textVertexShader.glsl", "textFragmentShader.glsl", true);

	try
	{
		if (FT_Init_FreeType(&ft))
		{
			throw runtime_error("Could not init FreeType Library");
		}
	}
	catch (const exception& e)
	{
		cerr << "ERROR::FREETYPE: " << e.what() << endl;
	}

}

TextRender::~TextRender()
{
}



vector<FT_Face> TextRender::SetUpTTF(FT_Face face, const char* filepathname, FT_Long face_index, FT_UInt pixel_width, FT_UInt pixel_height)
{
	TrueFont truefont
	{
		filepathname,
		face_index,
		pixel_width,
		pixel_height
	};

	try
	{
		if (FT_New_Face(ft, truefont.filepathname, truefont.face_index, &face))
		{
			throw runtime_error("Failed to load font from: " + string(truefont.filepathname));
		}
		else
		{
			FT_Set_Pixel_Sizes(face, truefont.pixel_width, truefont.pixel_height);

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		}
	}
	catch (const exception& e)
	{
		cerr << "ERROR::FREETYPE: " << e.what() << endl;
	}

	Faces.push_back(face);

	return Faces;
}

void TextRender::TexConfig()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void TextRender::LoadText(vector<FT_Face> Faces)
{
	for (int f = 0; f < Faces.size(); ++f)
	{
		FT_Face face = Faces[f];

		for (unsigned char c = 0; c < 128; ++c)
		{
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				cout << "ERROR::FREETYTPE: Failed to load Glyph" << endl;
				continue;
			}

			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			Character character = {
					texture,
					glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					static_cast<unsigned int>(face->glyph->advance.x)
			};

			Characters.insert(pair<char, Character>(c, character));

		}
		StyleLinker.insert(pair<string, map<GLchar, Character>>(face->family_name, Characters));

		glBindTexture(GL_TEXTURE_2D, 0);

		//FT_Done_Face(face);
	}
	//FT_Done_FreeType(ft);

	TexConfig();
}


void TextRender::RenderText(string styleName, Camera* cam, string text, float x, float y, float scale, glm::vec3 colour)
{
	m_shader->use();
	projection = cam->GetProjectionMatrix();
	viewMatrix = cam->GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(m_shader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(m_shader->ID, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3f(glGetUniformLocation(m_shader->ID, "textColour"), colour.x, colour.y, colour.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	auto it = StyleLinker.find(styleName);

	if (it != StyleLinker.end())
	{
		map<GLchar, Character> currentCharacters = it->second;

		string::const_iterator c;
		for (c = text.begin(); c != text.end(); ++c)
		{
			Character ch = currentCharacters[*c];

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;

			float vertices[6][4] = {
				{xpos, ypos + h, 0.0f, 0.0f },
				{xpos, ypos, 0.0f, 1.0f },
				{xpos + w, ypos, 1.0f, 1.0f },

				{xpos, ypos + h, 0.0f, 0.0f },
				{xpos + w, ypos, 1.0f, 1.0f },
				{xpos + w, ypos + h, 1.0f, 0.0f}
			};

			printf("RenderText: %s, textureId: %d\n", text.c_str(), ch.TextureID);
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			x += (ch.Advance >> 6) * scale;

		}

		currentCharacters.clear();
	}
	else
	{
		cout << "error" << endl;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}