#pragma once

#include <GL\glew.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <lodepng.h>

class GLTexture
{
public:
	GLTexture(std::string name, GLubyte color[4])
		: m_uiID(0)
		, m_strName(name)
		, m_uiWidth(1u)
		, m_uiHeight(1u)
	{
		if (color[3] == 0xFF)
			m_bTransparency = false;
		else
			m_bTransparency = true;

		load(color);
	}

	GLTexture(std::string name, unsigned short width, unsigned short height, unsigned char const * data, bool hasTransparency)
		: m_uiID(0)
		, m_strName(name)
		, m_uiWidth(width)
		, m_uiHeight(height)
		, m_bTransparency(hasTransparency)
	{		
		load(data);
	}

	GLTexture(std::string name, unsigned short width, unsigned short height, GLuint texID, bool hasTransparency)
		: m_uiID(texID)
		, m_strName(name)
		, m_uiWidth(width)
		, m_uiHeight(height)
		, m_bTransparency(hasTransparency)
	{
	}

	GLTexture(std::string png_filename, bool hasTransparency)
		: m_uiID(0)
		, m_uiWidth(0)
		, m_uiHeight(0)
		, m_bTransparency(hasTransparency)
	{
		m_strName = png_filename;
		
		// Load file and decode image.
		std::vector<unsigned char> image;

		unsigned error = lodepng::decode(image, m_uiWidth, m_uiHeight, m_strName);

		if (error != 0)
			std::cerr << "error " << error << ": " << lodepng_error_text(error) << std::endl;
		
		load(image.data());
	}

	~GLTexture()
	{
		glDeleteTextures(1, &m_uiID);
	}

	unsigned getWidth() { return m_uiWidth; }
	unsigned getHeight() { return m_uiHeight; }
	std::string getName() { return m_strName; }
	bool hasTransparency() { return m_bTransparency; }

	GLuint getTexture() { return m_uiID; }

private:
	GLuint m_uiID;
	std::string m_strName;
	unsigned m_uiWidth, m_uiHeight;
	bool m_bTransparency;

	void load(unsigned char const * data)
	{
		// Calculate number of mipmap levels for diffuse texture
		// this is taken straight from the spec for glTexStorage2D
		int diffuseMipMapLevels = (int)floor(log2((std::max)(m_uiWidth, m_uiHeight))) + 1;

		// Generate texture
		glCreateTextures(GL_TEXTURE_2D, 1, &m_uiID);
		glTextureStorage2D(m_uiID, diffuseMipMapLevels, GL_RGBA8, m_uiWidth, m_uiHeight);
		glTextureSubImage2D(m_uiID, 0, 0, 0, m_uiWidth, m_uiHeight, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);

		glGenerateTextureMipmap(m_uiID);

		glTextureParameteri(m_uiID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(m_uiID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_uiID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(m_uiID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLfloat fLargest;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
		glTextureParameterf(m_uiID, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
	}
};
