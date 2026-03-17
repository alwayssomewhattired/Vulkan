#pragma once

#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>

#include "glm_config.h"
#include "items/ItemInterface.h"

#include <vector>
class Animation
{
public:

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

	struct AnimationChannel
	{
		std::vector<PositionKey> positions;
		std::vector<RotationKey> rotations;
		std::vector<ScaleKey> scales;
	};

	struct AnimationData
	{
		float duration;
		float ticksPerSecond;

		std::vector<AnimationChannel> channels;
	};

	void initialize();
	void update(float deltaTime);

private:

	ItemInterface::Skeleton* skeleton;
	AnimationData* animation;

	float currentTime = 0.0f;

	std::vector<glm::mat4> boneMatrices;
	std::vector<glm::mat4> localTransforms;
	std::vector<glm::mat4> globalTransforms;

	glm::vec3 InterpolatePosition(const AnimationChannel& channel, float time);
	glm::quat InterpolateRotation(const AnimationChannel& channel, float time);
	glm::vec3 InterpolateScale(const AnimationChannel& channel, float time);

	void computeLocalTransforms();
	void computeGlobalTransforms();
	void computeFinalMatrices();

};

