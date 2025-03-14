#include "SkinnedMesh.hpp"
#include <glm/gtc/quaternion.hpp>
#include <spdlog/spdlog.h>

glm::vec3 convertVector(aiVector3D& v) {
	return glm::vec3(v.x, v.y, v.z);
}

glm::quat convertQuaternion(aiQuaternion& q) {
	return glm::quat(q.w, q.x, q.y, q.z);
}

void SkinnedMesh::parseAnimation(const aiScene* scene) {
	std::unordered_map<std::string, int> boneIndices{};

	for (int b = 0; b < mBones.size(); b++) {
		boneIndices[mBones[b].name] = b;
	}

	for (int a = 0; a < scene->mNumAnimations; a++) {
		aiAnimation* anim = scene->mAnimations[a];

		double timeScale = 1.0 / anim->mTicksPerSecond;

		SkinnedMeshAnimation animation{ anim->mDuration / anim->mTicksPerSecond };

		for (int c = 0; c < anim->mNumChannels; c++) {
			aiNodeAnim* nodeAnim = anim->mChannels[c];

			if (boneIndices.find(nodeAnim->mNodeName.C_Str()) == boneIndices.end()) {
				continue;
			}
			int boneIndex = boneIndices[nodeAnim->mNodeName.C_Str()];

			BoneClip clip{ boneIndex };

			for (int k = 0; k < nodeAnim->mNumPositionKeys; k++) {
				aiVectorKey& key = nodeAnim->mPositionKeys[k];
				clip.positionFrames.emplace_back(timeScale * key.mTime, convertVector(key.mValue));
			}

			for (int k = 0; k < nodeAnim->mNumScalingKeys; k++) {
				aiVectorKey& key = nodeAnim->mScalingKeys[k];
				clip.scaleFrames.emplace_back(timeScale * key.mTime, convertVector(key.mValue));
			}

			for (int k = 0; k < nodeAnim->mNumRotationKeys; k++) {
				aiQuatKey& key = nodeAnim->mRotationKeys[k];
				clip.rotationFrames.emplace_back(timeScale * key.mTime, convertQuaternion(key.mValue));
			}

			animation.clips.push_back(clip);
		}

		mAnimations[anim->mName.C_Str()] = animation;
		spdlog::info("Animation name: {}", anim->mName.C_Str());
	}
}

void SkinnedMesh::animate(double t) {
	if (mAnimations.find(mCurrentAnimation) == mAnimations.end()) return;

	SkinnedMeshAnimation& animation = mAnimations[mCurrentAnimation];

	double relT = animation.duration * glm::fract(t / animation.duration);
	for (const BoneClip& clip : animation.clips) {
		int i = 0;
		float fac;

		glm::vec3 position = { 0, 0, 0 };
		glm::vec3 scale = { 1, 1, 1 };
		glm::quat rotation = { 1, 0, 0, 0 };

		if (clip.positionFrames.size() > 1) {
			for (i = 0; i < clip.positionFrames.size() - 2; i++) {
				if (clip.positionFrames[i + 1].first > relT) break;
			}
			fac = glm::clamp((relT - clip.positionFrames[i].first) / (clip.positionFrames[i + 1].first - clip.positionFrames[i].first), 0.0, 1.0);
			position = (1.0f - fac) * clip.positionFrames[i].second + fac * clip.positionFrames[i + 1].second;
		}
		else {
			position = clip.positionFrames[0].second;
		}

		if (clip.scaleFrames.size() > 1) {
			for (i = 0; i < clip.scaleFrames.size() - 2; i++) {
				if (clip.scaleFrames[i + 1].first > relT) break;
			}
			fac = glm::clamp((relT - clip.scaleFrames[i].first) / (clip.scaleFrames[i + 1].first - clip.scaleFrames[i].first), 0.0, 1.0);
			scale = (1.0f - fac) * clip.scaleFrames[i].second + fac * clip.scaleFrames[i + 1].second;
		}
		else {
			scale = clip.scaleFrames[0].second;
		}

		if (clip.rotationFrames.size() > 1) {
			for (i = 0; i < clip.rotationFrames.size() - 2; i++) {
				if (clip.rotationFrames[i + 1].first > relT) break;
			}
			fac = glm::clamp((relT - clip.rotationFrames[i].first) / (clip.rotationFrames[i + 1].first - clip.rotationFrames[i].first), 0.0, 1.0);
			rotation = glm::slerp(clip.rotationFrames[i].second, clip.rotationFrames[i + 1].second, fac);
		}
		else {
			rotation = clip.rotationFrames[0].second;
		}

		mBones[clip.boneIndex].relativeMatrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), position) * glm::mat4_cast(rotation), scale);
	}
}

void SkinnedMesh::setAnimation(std::string name) {
	mCurrentAnimation = name;
}
