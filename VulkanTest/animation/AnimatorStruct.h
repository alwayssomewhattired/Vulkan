#pragma once

#include "../glm_config.h"

#include <vector>
#include <unordered_map>
#include <string>

struct AnimatorStruct {
	
	struct Bone {
		int parentIndex;
		std::string name;
		glm::mat4 inverseBindMatrix;
		glm::mat4 localBindTransform;
	};

	struct Skeleton {
		// | bone-name to bone-index
		std::unordered_map<std::string, int> boneMap;
		//std::vector<Bone> bones;
		glm::mat4 globalInverseTransform;

		// | starting index to object in GlobalSkeleton
		uint32_t offset;

		// | size of object stored in GlobalSkeleton
		uint32_t count;
	};

	struct GlobalSkeleton {
		// | bone-name to bone-index
		//std::unordered_map<std::string, int> boneMap;

		std::vector<Bone> bones;
		//std::vector<glm::mat4> globalInverseTransform;
	};

	struct PositionKey
	{
		float time;
		glm::vec3 value;
	};
	struct RotationKey
	{
		float time;
		glm::quat value;
	};
	struct ScaleKey
	{
		float time;
		glm::vec3 value;
	};

	struct AnimatorChannel
	{
		std::vector<PositionKey> positions;
		std::vector<RotationKey> rotations;
		std::vector<ScaleKey> scales;
	};

	struct AnimatorData
	{
		float duration = 0.0f;
		float ticksPerSecond = 25.0f;

		std::unordered_map<std::string, AnimatorChannel> channels;
	};
};