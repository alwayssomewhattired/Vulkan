#pragma once

#include "../glm_config.h"

#include <vector>
#include <unordered_map>
#include <string>

struct AnimatorStruct {
	
	struct Bone {
		int parentIndex;
		glm::mat4 inverseBindMatrix;
	};

	struct Skeleton {
		// | bone-name to bone-index
		std::unordered_map<std::string, int> boneMap;
		std::vector<Bone> bones;
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
		float duration;
		float ticksPerSecond;

		std::vector<AnimatorChannel> channels;
	};
};