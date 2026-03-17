#include "Animation.h"

Animation::Animation() {};

void Animation::initialize() {
	int count = skeleton->bones.size();

	boneMatrices.resize(count);
	localTransforms.resize(count);
	globalTransforms.resize(count);
}

glm::vec3 Animation::InterpolatePosition(const AnimationChannel& channel, float time)
{
	if (channel.positions.size() == 1)
		return channel.positions[0].value;

	for (size_t i = 0; i < channel.positions.size() - 1; i++)
	{
		if (time < channel.positions[i + 1].time)
		{
			auto& a = channel.positions[i];
			auto& b = channel.positions[i + 1];

			float t = (time - a.time) / (b.time - a.time);
			return glm::mix(a.value, b.value, t);
		}
	}

	return channel.positions.back().value;
}

void Animation::computeLocalTransforms()
{
	int boneCount = skeleton->bones.size();

	for (int i = 0; i < boneCount; i++)
	{
		const AnimationChannel& channel = animation->channels[i];

		glm::vec3 pos = InterpolatePosition(channel, currentTime);
		glm::quat rot = InterpolateRotation(channel, currentTime);
		glm::vec3 scale = InterpolateScale(channel, currentTime);

		glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
		glm::mat4 R = glm::toMat4(rot);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

		localTransforms[i] = T * R * S;
	}

}

void Animation::computeGlobalTransforms()
{
	int boneCount = skeleton->bones.size();

	for (int i = 0; i < boneCount; i++)
	{
		int parent = skeleton->bones[i].parentIndex;

		if (parent < 0)
			globalTransforms[i] = localTransforms[i];
		else
			globalTransforms[i] = globalTransforms[parent] * localTransforms[i];
	}
}

void Animation::computeFinalMatrices()
{
	int boneCount = skeleton->bones.size();

	for (int i = 0; i < boneCount; i++)
	{
		boneMatrices[i] = globalTransforms[i] * skeleton->bones[i].inverseBindMatrix;
	}
}

void Animation::update(float deltaTime) {
	float tps = animation->ticksPerSecond != 0 ? animation->ticksPerSecond : 25.0f;

	currentTime += deltaTime * tps;
	currentTime = fmod(currentTime, animation->duration);

	computeLocalTransforms();
	computeGlobalTransforms();
	computeFinalMatrices();
}