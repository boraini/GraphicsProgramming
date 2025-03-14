#pragma once

#include <functional>
#include <string>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

typedef std::function<void(int, unsigned char*)> FetchDataHandler;

void fetch_image(std::string root, std::string path, std::function<void(unsigned char*, int, int, int)> handler);
void fetch_assimp_scene(std::string root, std::string path, unsigned int postprocessingFlags, std::function<void(std::string, const aiScene*)> handler);
void fetch_data(std::string root, std::string path, FetchDataHandler handler);
