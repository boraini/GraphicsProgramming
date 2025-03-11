#pragma once

#define STB_IMAGE_IMPLEMENTATION

#include <unordered_map>
#include <glad/glad.h>
#include <string>
#include <memory>

class MaterialManager {
public:
	GLuint getTexture(std::string path);
	void unloadTextures();

private:
	std::unordered_map<std::string, GLuint> mTextures;
};

extern MaterialManager* globalMaterialManager;
