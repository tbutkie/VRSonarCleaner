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
		glGenTextures(1, &m_uiID);
		glBindTexture(GL_TEXTURE_2D, m_uiID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			m_uiWidth,
			m_uiHeight,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			&image[0]);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	~Texture()
	{
		glDeleteTextures(1, &m_uiID);
	}

	unsigned getWidth() { return m_uiWidth; }
	unsigned getHeight() { return m_uiHeight; }

	void activate()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiID);
	}

	void deactivate()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

private:
	GLuint m_uiID;
	std::string m_strFileName;
	unsigned m_uiWidth, m_uiHeight;
};
