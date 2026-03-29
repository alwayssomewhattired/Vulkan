#include "Animator.h"

Animator::Animator() {};

void Animator::initialize(ItemInterface& item) {
	int count = globalSkeleton.bones.size();

	globalBoneMatrices.resize(count);
	item.localTransforms.resize(count);
	item.globalTransforms.resize(count);
}

glm::vec3 Animator::InterpolatePosition(const AnimatorStruct::AnimatorChannel& channel, float time)
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

glm::quat Animator::InterpolateRotation(const AnimatorStruct::AnimatorChannel& channel, float time)
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

glm::vec3 Animator::InterpolateScale(const AnimatorStruct::AnimatorChannel& channel, float time)
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

void Animator::computeLocalTransforms(ItemInterface& item)
{
	const auto boneCount = item.skeleton.count;
	const auto offset = item.skeleton.offset;
	const auto globalBoneCount = globalSkeleton.bones.size();

	for (int i = offset; i < offset + boneCount; i++)
	{
		int j = i % boneCount;

		const std::string name = globalSkeleton.bones[i].name;
		const AnimatorStruct::AnimatorChannel& channel = item.animatorData.channels[name];
		const float currentTime = item.currentTimeAnim;

		const bool hasAnimation =
			!channel.positions.empty() ||
			!channel.rotations.empty() ||
			!channel.scales.empty();
		if (hasAnimation)
		{
			glm::vec3 pos = InterpolatePosition(channel, currentTime);
			glm::quat rot = InterpolateRotation(channel, currentTime);
			glm::vec3 scale = InterpolateScale(channel, currentTime);

			glm::mat4 T = glm::translate(glm::mat4(1.0f), pos);
			glm::mat4 R = glm::toMat4(rot);
			glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

			item.localTransforms[j] = T * R * S;
		}
		else {
			item.localTransforms[j] = globalSkeleton.bones[i].localBindTransform;
		}

	}

}


void Animator::computeGlobalTransforms(ItemInterface& item) {
	const auto boneCount = item.skeleton.count;
	const auto offset = item.skeleton.offset;
	const auto globalBoneCount = globalSkeleton.bones.size();

	for (int i = offset; i < offset + boneCount; i++) {
		int j = i % boneCount;
		int parent = globalSkeleton.bones[i].parentIndex;

		if (parent == -1) {
			item.globalTransforms[j] = item.localTransforms[j];
		}
		else {
			item.globalTransforms[j] = item.globalTransforms[parent] * item.localTransforms[j];
		}
	}
}

void Animator::computeFinalMatrices(ItemInterface& item)
{
	const auto boneCount = item.skeleton.count;
	const auto offset = item.skeleton.offset;


	for (int i = offset; i < offset + boneCount; i++)
	{
		int j = i % boneCount;
		globalBoneMatrices[i] = item.globalTransforms[j] * globalSkeleton.bones[i].inverseBindMatrix;
	}
}

void Animator::update(float deltaTime, ItemInterface& item) {

	if (item.skeleton.count == 0)
		return;

	float tps = item.animatorData.ticksPerSecond != 0.0f ? item.animatorData.ticksPerSecond : 25.0f;

	item.currentTimeAnim += deltaTime * tps;
	item.currentTimeAnim = fmod(item.currentTimeAnim, item.animatorData.duration);
	computeLocalTransforms(item);
	computeGlobalTransforms(item);
	computeFinalMatrices(item);
}
