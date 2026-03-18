#include "Animator.h"

Animator::Animator() {};

void Animator::initialize(ItemInterface& item) {
	int count = skeleton->bones.size();

	item.boneMatrices.resize(count);
	localTransforms.resize(count);
	globalTransforms.resize(count);
}

glm::vec3 Animator::InterpolatePosition(const AnimatorChannel& channel, float time)
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

glm::quat Animator::InterpolateRotation(const AnimatorChannel& channel, float time)
{
	if (channel.rotations.size() == 1)
		return glm::normalize(channel.rotations[0].value);

	for (size_t i = 0; i < channel.rotations.size() - 1; i++)
	{
		if (time < channel.rotations[i + 1].time)
		{
			const auto& a = channel.rotations[i];
			const auto& b = channel.rotations[i + 1];

			float t = (time - a.time) / (b.time - a.time);

			glm::quat q1 = a.value;
			glm::quat q2 = b.value;

			if (glm::dot(q1, q2) < 0.0f)
				q2 = -q2;

			return glm::normalize(glm::slerp(q1, q2, t));
		}
	}

	return glm::normalize(channel.rotations.back().value);
}

glm::vec3 Animator::InterpolateScale(const AnimatorChannel& channel, float time)
{
	if (channel.scales.size() == 1)
		return channel.scales[0].value;

	for (size_t i = 0; i < channel.scales.size() - 1; i++)
	{
		if (time < channel.scales[i + 1].time)
		{
			const auto& a = channel.scales[i];
			const auto& b = channel.scales[i+1];

			float t = (time - a.time) / (b.time - a.time);
			return glm::mix(a.value, b.value, t);
		}
	}

	return channel.scales.back().value;
}

void Animator::computeLocalTransforms()
{
	int boneCount = skeleton->bones.size();

	for (int i = 0; i < boneCount; i++)
	{
		const AnimatorChannel& channel = AnimatorData->channels[i];

		glm::vec3 pos = InterpolatePosition(channel, currentTime);
		glm::quat rot = InterpolateRotation(channel, currentTime);
		glm::vec3 scale = InterpolateScale(channel, currentTime);

		glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
		glm::mat4 R = glm::toMat4(rot);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

		localTransforms[i] = T * R * S;
	}

}

void Animator::computeGlobalTransforms()
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

void Animator::computeFinalMatrices(ItemInterface& item)
{
	int boneCount = skeleton->bones.size();

	for (int i = 0; i < boneCount; i++)
	{
		item.boneMatrices[i] = globalTransforms[i] * skeleton->bones[i].inverseBindMatrix;
	}
}

void Animator::update(float deltaTime, ItemInterface& item) {
	float tps = AnimatorData->ticksPerSecond != 0 ? AnimatorData->ticksPerSecond : 25.0f;

	currentTime += deltaTime * tps;
	currentTime = fmod(currentTime, AnimatorData->duration);

	computeLocalTransforms();
	computeGlobalTransforms();
	computeFinalMatrices(item);
}