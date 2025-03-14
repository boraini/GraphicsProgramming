#pragma once

#include "opengl.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <assimp/texture.h>
#include <stb_image.h>

class MaterialManager {
public:
	GLuint getTexture(std::string path);
	void addTexture(std::string path, const aiTexture* texture);
	void unloadTextures();

private:
	std::unordered_map<std::string, GLuint> mTextures;
};

extern MaterialManager* globalMaterialManager;
