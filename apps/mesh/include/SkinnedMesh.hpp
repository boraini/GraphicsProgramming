#pragma once

#include "opengl.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include "shader.hpp"

struct SkinnedVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::ivec4 bone;
    glm::vec4 influence;
};

struct Mesh {
    GLuint vertexArray;
	GLuint numIndices;
    std::string diffuseTexture;
    std::string specularTexture;
};

struct Bone {
	// Indicates the geometric parent index.
	// Always smaller than the current bone so that computation of the whole skeleton can be done with a linear pass.
	int parent;
	// Indicates where to put the matrix relative to the armature when building the vertex shader uniform data.
	int matrixIndex;
	// Matrix relative to the parent bone, or the armature in case of the root bone.
	glm::mat4 relativeMatrix;
	// Matrix the bone is actually offset from the emulated child node position.
	glm::mat4 offsetMatrix;
	// Name of the bone
	std::string name;
};

// Object class that contains a set of meshes that are deformed by some bones.
// It can be loaded from any file format that Assimp can extract an armature and bones from.
class SkinnedMesh {
public:
	// Load from the given file.
	SkinnedMesh(std::string filename);
	// Draw each deformed mesh using OpenGL.
    void draw(glm::mat4 projection, glm::mat4 cameraInverse, glm::mat4 matrix);
	// Get a reference to a bone by its name.
	Bone& getBone(std::string name);
private:
	void parse(const std::string assetPath, const aiScene* scene);
	void parse(std::string assetPath, const aiScene* scene, const aiMesh* mesh, const std::vector<std::vector<std::pair<float, int>>>& sortedWeights);
    void createBoneMatrices(int parentIndex, const aiNode* currentBone, std::unordered_map<const aiNode*, const aiBone*>& nodeBones, std::unordered_map<const aiNode*, int>& boneMatrixIndices);

	std::vector<Bone> mBones;
	std::vector<glm::mat4> mBoneNodeMatrices;
	std::vector<glm::mat4> mBoneMatrices;
    std::vector<Mesh> mSkinnedMeshes;

	inline static std::unique_ptr<Shader> mShader;
};
