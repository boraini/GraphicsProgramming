#pragma once

#define STB_IMAGE_IMPLEMENTATION

#include "opengl.hpp"
#include <unordered_map>
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
