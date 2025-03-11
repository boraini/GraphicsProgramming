#include "MaterialManager.hpp"

#include <unordered_map>
#include <glad/glad.h>
#include <string>
#include <stb_image.h>
#include <spdlog/spdlog.h>

MaterialManager* globalMaterialManager;

GLuint MaterialManager::getTexture(std::string path) {
	if (mTextures.find(path) == mTextures.end()) {
		GLenum format;
		GLuint texture;
		std::string mode;

		// Load image from file
		int width, height, channels;
		unsigned char* image = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (!image) spdlog::critical("Failed to Load Texture {}!", path.c_str());

		// Set the Correct Channel Format
		switch (channels)
		{
		case 1: format = GL_ALPHA;     break;
		case 2: format = GL_LUMINANCE; break;
		case 3: format = GL_RGB;       break;
		case 4: format = GL_RGBA;      break;
		}

		// Write texture to the GPU
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, format,
			width, height, 0, format, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

		mTextures[path] = texture;
	}

	return mTextures[path];
}

void MaterialManager::unloadTextures() {
	for (std::pair<std::string, GLuint> kvp : mTextures) {
		glDeleteTextures(1, &kvp.second);
	}

	mTextures.clear();
}
