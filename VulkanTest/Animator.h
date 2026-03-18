#pragma once

#include "glm_config.h"
//#include <glm/fwd.hpp>
//#include <glm/glm.hpp>
//#include <glm/gtc/quaternion.hpp>
//#include <glm/gtc/matrix_transform.hpp>

#include "items/ItemInterface.h"

#include <vector>

class Animator
{
public:

	Animator();

	void initialize(ItemInterface& item);
	void update(float deltaTime, ItemInterface& item);

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

	AnimatorData* AnimatorData;

private:

	ItemInterface::Skeleton* skeleton;

	float currentTime = 0.0f;

	std::vector<glm::mat4> localTransforms;
	std::vector<glm::mat4> globalTransforms;

	glm::vec3 InterpolatePosition(const AnimatorChannel& channel, float time);
	glm::quat InterpolateRotation(const AnimatorChannel& channel, float time);
	glm::vec3 InterpolateScale(const AnimatorChannel& channel, float time);

	void computeLocalTransforms();
	void computeGlobalTransforms();
	void computeFinalMatrices(ItemInterface& item);

};

