#pragma once

#include "glm_config.h"
//#include <glm/fwd.hpp>
//#include <glm/glm.hpp>
//#include <glm/gtc/quaternion.hpp>
//#include <glm/gtc/matrix_transform.hpp>

#include "items/ItemInterface.h"
#include "animation/AnimatorStruct.h"

#include <vector>
#include <iostream>

class Animator
{
public:

	Animator();

	void initialize(ItemInterface& item);
	void update(float deltaTime, ItemInterface& item);




private:

	glm::vec3 InterpolatePosition(const AnimatorStruct::AnimatorChannel& channel, float time);
	glm::quat InterpolateRotation(const AnimatorStruct::AnimatorChannel& channel, float time);
	glm::vec3 InterpolateScale(const AnimatorStruct::AnimatorChannel& channel, float time);

	void computeLocalTransforms(ItemInterface& item);
	void computeGlobalTransforms(ItemInterface& item);
	void computeFinalMatrices(ItemInterface& item);

};

