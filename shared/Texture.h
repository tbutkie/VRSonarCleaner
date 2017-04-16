#include <GL\glew.h>
#include <iostream>
#include <string>
#include "shared/lodepng.h"

class Texture
{
public:
	Texture(std::string png_filename)
		: m_uiID(0)
		, m_uiWidth(0)
		, m_uiHeight(0)
	{
		m_strFileName = png_filename;
		
		// Load file and decode image.
		std::vector<unsigned char> image;

		unsigned error = lodepng::decode(image, m_uiWidth, m_uiHeight, m_strFileName);

		if (error != 0)
			std::cerr << "error " << error << ": " << lodepng_error_text(error) << std::endl;

		// Generate texture
		glCreateTextures(GL_TEXTURE_2D, 1, &m_uiID);
		glTextureStorage2D(m_uiID, 1, GL_RGBA8, m_uiWidth, m_uiHeight);
		glTextureSubImage2D(m_uiID, 0, 0, 0, m_uiWidth, m_uiHeight, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

		glGenerateTextureMipmap(m_uiID);

		glTextureParameteri(m_uiID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_uiID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_uiID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_uiID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	~Texture()
	{
		glDeleteTextures(1, &m_uiID);
	}

	unsigned getWidth() { return m_uiWidth; }
	unsigned getHeight() { return m_uiHeight; }
	std::string getName() { return m_strFileName; }

	GLuint getTexture() { return m_uiID; }

private:
	GLuint m_uiID;
	std::string m_strFileName;
	unsigned m_uiWidth, m_uiHeight;
};
