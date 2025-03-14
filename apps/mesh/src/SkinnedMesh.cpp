#include "SkinnedMesh.hpp"

#include <vector>
#include <stack>
#include <iostream>

#include <assimp/postprocess.h>
#include <spdlog/spdlog.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MaterialManager.hpp"
#include "fetch.hpp"
#include "shaders.hpp"

constexpr auto BONES_PER_VERTEX = 4;

struct WeightSmallerComparator
{
    bool operator()(const std::pair<float, int>& s1, std::pair<float, int>& s2)
    {
        return s1.first < s2.first && s1.second == s2.second;
    }
};

glm::mat4 convertMatrix(const aiMatrix4x4& aiMat)
{
    return {
    aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
    aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
    aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
    aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
    };
}

SkinnedMesh::SkinnedMesh(std::string filename) {
    if (mShader == nullptr) {
        mShader = std::make_unique<Shader>();
        mShader->addSource("SkinnedMesh.vert", GL_VERTEX_SHADER, SkinnedMesh_vert_count, SkinnedMesh_vert, SkinnedMesh_vert_lens);
        mShader->addSource("SkinnedMesh.frag", GL_FRAGMENT_SHADER, SkinnedMesh_frag_count, SkinnedMesh_frag, SkinnedMesh_frag_lens);
        mShader->link();
    }

    fetch_assimp_scene(
        COMMON_ASSETS_DIR,
        filename,
        aiProcessPreset_TargetRealtime_MaxQuality |
        aiProcess_OptimizeGraph |
        aiProcess_FlipUVs |
        aiProcess_PopulateArmatureData,
        [this](std::string assetPath, const aiScene* scene) { parse(assetPath, scene); }
    );
}

void SkinnedMesh::parse(std::string assetPath, const aiScene* scene) {
    aiNode* armature = nullptr;
    std::vector<aiMesh*> meshesToParse{};
    std::unordered_map<const aiNode*, int> boneMatrixIndices;
    std::unordered_map<const aiNode*, const aiBone*> nodeBones;
    std::vector<aiBone*> foundBones;
    int boneMatrixCounter = 0;
    for (int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        bool alreadyListed = false;

        if (armature == nullptr && mesh->HasBones()) {
            armature = scene->mMeshes[i]->mBones[0]->mArmature;
        }

        for (int b = 0; b < mesh->mNumBones; b++) {
            aiBone* bone = mesh->mBones[b];
            if (armature == mesh->mBones[b]->mArmature) {
                if (!alreadyListed) {
                    meshesToParse.push_back(mesh);
                    alreadyListed = true;
                }

                if (boneMatrixIndices.find(bone->mNode) == boneMatrixIndices.end()) {
                    boneMatrixIndices[bone->mNode] = boneMatrixCounter++;
                    foundBones.push_back(bone);
                    nodeBones[bone->mNode] = bone;
                }
            }
        }
    }

    if (armature == nullptr) {
        spdlog::warn("No armature was found. Object will be empty.");
        return;
    }

    for (aiBone* foundBone : foundBones) {
        aiNode* currentNode = foundBone->mNode;
        while (currentNode != nullptr) {
            if (boneMatrixIndices.find(currentNode) == boneMatrixIndices.end()) {
                boneMatrixIndices[currentNode] = boneMatrixCounter++;
                nodeBones[currentNode] = foundBone;
            }
            if (currentNode == armature) {
                break;
            }
            currentNode = currentNode->mParent;
        }
    }

    mBoneMatrices.resize(boneMatrixCounter);
    mBoneNodeMatrices.resize(boneMatrixCounter);
    createBoneMatrices(0, armature, nodeBones, boneMatrixIndices);

    for (int i = 0; i < meshesToParse.size(); i++) {
        aiMesh* mesh = scene->mMeshes[i];

        std::vector<std::vector<std::pair<float, int>>> boneInfluencesPerVertex{};

        boneInfluencesPerVertex.resize(mesh->mNumVertices);

        for (int b = 0; b < mesh->mNumBones; b++) {
            aiBone* bone = mesh->mBones[b];
            for (int bv = 0; bv < bone->mNumWeights; bv++) {
                std::vector<std::pair<float, int>>& currentVertex = boneInfluencesPerVertex[bone->mWeights[bv].mVertexId];
                if (currentVertex.size() < BONES_PER_VERTEX - 1) {
                    // add and don't sort
                    currentVertex.emplace_back(bone->mWeights[bv].mWeight, boneMatrixIndices[bone->mNode]);
                }
                else if (currentVertex.size() == BONES_PER_VERTEX - 1) {
                    // add and build heap
                    currentVertex.emplace_back(bone->mWeights[bv].mWeight, boneMatrixIndices[bone->mNode]);
                    std::make_heap(currentVertex.begin(), currentVertex.end(), WeightSmallerComparator());
                }
                else if (currentVertex.front().first < bone->mWeights[bv].mWeight) {
                    // add only if there is more weight than the current maximum, and remove the smallest in that case
                    std::pop_heap(currentVertex.begin(), currentVertex.end(), WeightSmallerComparator());
                    currentVertex.pop_back();
                    currentVertex.emplace_back(bone->mWeights[bv].mWeight, boneMatrixIndices[bone->mNode]);
                    std::push_heap(currentVertex.begin(), currentVertex.end());
                }
            }
        }

        parse(assetPath, scene, mesh, boneInfluencesPerVertex);
    }

    std::stack<int> current{};
    current.push(0);
    for (int i = 0; i < mBones.size(); i++) {
        if (mBones[i].parent > current.top()) {
            current.push(i - 1);
        }
        else {
            while (current.top() > mBones[i].parent) {
                current.pop();
            }
        }

        for (int j = 0; j < current.size() - 1; j++) {
            std::cout << "|";
        }

        if (current.size() > 1) std::cout << "L";

        std::cout << mBones[i].name << " -> " << mBones[i].parent << std::endl;
    }
}

void SkinnedMesh::parse(std::string assetPath, const aiScene* scene, const aiMesh* mesh, const std::vector<std::vector<std::pair<float, int>>>& sortedWeights) {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    std::string diffuseTexture;
    std::string specularTexture;

    for (int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
        aiString str;
        material->GetTexture(aiTextureType_DIFFUSE, i, &str);
        diffuseTexture = assetPath + "/" + str.C_Str();
        globalMaterialManager->getTexture(diffuseTexture);
    }

    for (int i = 0; i < material->GetTextureCount(aiTextureType_SPECULAR); i++) {
        aiString str;
        material->GetTexture(aiTextureType_SPECULAR, i, &str);
        specularTexture = assetPath + "/" + str.C_Str();
        globalMaterialManager->getTexture(specularTexture);
    }

    std::vector<SkinnedVertex> vertexBuffer{};
    vertexBuffer.resize(mesh->mNumVertices);
    
    for (int i = 0; i < vertexBuffer.size(); i++) {
        SkinnedVertex& v = vertexBuffer[i];
        v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        v.uv = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

        for (int j = 0; j < sortedWeights[i].size(); j++) {
            v.influence[j] = sortedWeights[i][j].first;
            v.bone[j] = sortedWeights[i][j].second;
        }

        for (int j = sortedWeights[i].size(); j < 4; j++) {
            v.influence[j] = 0.0;
            v.bone[j] = 0;
        }
    }

    std::vector<GLuint> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
            indices.push_back(mesh->mFaces[i].mIndices[j]);

    GLuint vao, vbo, vi;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(SkinnedVertex), &vertexBuffer.front(), GL_STATIC_DRAW);

    glGenBuffers(1, &vi);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vi);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices.front(), GL_STATIC_DRAW);

    glVertexAttribPointer(mShader->getAttribute("position"), 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, position));
    glVertexAttribPointer(mShader->getAttribute("normal"), 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, normal));
    glVertexAttribPointer(mShader->getAttribute("uv"), 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, uv));
    glVertexAttribIPointer(mShader->getAttribute("bone"), 4, GL_INT, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, bone));
    glVertexAttribPointer(mShader->getAttribute("influence"), 4, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (void*)offsetof(SkinnedVertex, influence));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
    //glDeleteBuffers(1, &vbo);

    mSkinnedMeshes.push_back({ vao, (GLuint)indices.size(), diffuseTexture, specularTexture });
}

void SkinnedMesh::createBoneMatrices(int parentIndex, const aiNode* currentBone, std::unordered_map<const aiNode*, const aiBone*>& nodeBones, std::unordered_map<const aiNode*, int>& boneMatrixIndices) {
    // printf("Handling bone: %s\n", currentBone->mName.data);
    if (currentBone == nullptr) return;
    mBones.push_back({ parentIndex, boneMatrixIndices[currentBone], convertMatrix(currentBone->mTransformation), convertMatrix(nodeBones[currentBone]->mOffsetMatrix), std::string(currentBone->mName.data) });
    int myIndex = mBones.size() - 1;
    for (int i = 0; i < currentBone->mNumChildren; i++) {
        if (boneMatrixIndices.find(currentBone->mChildren[i]) != boneMatrixIndices.end()) {
            createBoneMatrices(myIndex, currentBone->mChildren[i], nodeBones, boneMatrixIndices);
        }
    }
}

void SkinnedMesh::draw(glm::mat4 projection, glm::mat4 cameraInverse, glm::mat4 matrix) {
    // Not loaded yet.
    if (mSkinnedMeshes.size() == 0) return;

    mShader->use();

    mBoneNodeMatrices[0] = glm::identity<glm::mat4>();
    glm::mat4 globalInverse = glm::inverse(mBones[0].relativeMatrix);
    for (int i = 0; i < mBones.size(); i++) {
        mBoneNodeMatrices[i] = mBoneNodeMatrices[mBones[i].parent] * mBones[i].relativeMatrix;
        mBoneMatrices[mBones[i].matrixIndex] = globalInverse * mBoneNodeMatrices[i] * mBones[i].offsetMatrix;
    }

    glUniformMatrix4fv(SkinnedMesh::mShader->getUniform("projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(SkinnedMesh::mShader->getUniform("cameraInverseMatrix"), 1, GL_FALSE, glm::value_ptr(cameraInverse));
    glUniformMatrix4fv(SkinnedMesh::mShader->getUniform("objectMatrix"), 1, GL_FALSE, glm::value_ptr(matrix));
    glUniformMatrix4fv(SkinnedMesh::mShader->getUniform("boneMatrices"), mBoneMatrices.size(), GL_FALSE, glm::value_ptr(mBoneMatrices.front()));

    for (auto& mesh : mSkinnedMeshes) {
        glBindVertexArray(mesh.vertexArray);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, globalMaterialManager->getTexture(mesh.diffuseTexture));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, globalMaterialManager->getTexture(mesh.specularTexture));

        glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, 0);
    }
}

Bone& SkinnedMesh::getBone(std::string name) {
    for (Bone& b : mBones) {
        if (b.name == name) {
            return b;
        }
    }

    spdlog::warn("Bone \"{}\" not found!", name);
    return mBones[0];
}
