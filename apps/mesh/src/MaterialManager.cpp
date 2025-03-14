#include "MaterialManager.hpp"

#include <fetch.hpp>
#include <unordered_map>
#include <string>
#include <spdlog/spdlog.h>

MaterialManager* globalMaterialManager;

GLuint MaterialManager::getTexture(std::string path) {
	if (mTextures.find(path) == mTextures.end()) {
		GLuint texture;

		// Write texture to the GPU
		glGenTextures(1, &texture);
		mTextures[path] = texture;

		// This should happen between two frames in an async runtime, and will be synchronous in native.
		fetch_image("", path, [texture](unsigned char* data, int width, int height, int channels) {
			GLenum format;

			// Set the Correct Channel Format
			switch (channels)
			{
			case 1: format = GL_ALPHA;     break;
			case 2: format = GL_LUMINANCE; break;
			case 3: format = GL_RGB;       break;
			case 4: format = GL_RGBA;      break;
			}

			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, format,
				width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		});
	}

	return mTextures[path];
}

void MaterialManager::unloadTextures() {
	for (std::pair<std::string, GLuint> kvp : mTextures) {
		glDeleteTextures(1, &kvp.second);
	}

	mTextures.clear();
}
