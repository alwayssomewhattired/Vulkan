#include "Animator.h"

Animator::Animator() {};

void Animator::initialize(ItemInterface& item) {
	int count = item.skeleton.bones.size();

	item.boneMatrices.resize(count);
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
	const int boneCount = item.skeleton.bones.size();

	for (int i = 0; i < boneCount; i++)
	{
		const std::string name = item.skeleton.bones[i].name;
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

			item.localTransforms[i] = T * R * S;
		}
		else {
			item.localTransforms[i] = item.skeleton.bones[i].localBindTransform;
		}


		//const glm::mat4& m = item.localTransforms[i];
		//std::cout << m[0][0] << " " << m[1][1] << " " << m[2][2] << " " << m[3][3] << "\n";
	}

}

glm::mat4 computeGlobal(int i, ItemInterface& item) {

	int parent = item.skeleton.bones[i].parentIndex;

	if (parent < 0)
		return item.localTransforms[i];

	return computeGlobal(parent, item) * item.localTransforms[i];
}

void Animator::computeGlobalTransforms(ItemInterface& item)
{
	int boneCount = item.skeleton.bones.size();
	for (int i = 0; i < boneCount; i++) {
		item.globalTransforms[i] = computeGlobal(i, item);

		//const glm::mat4& m = item.globalTransforms[i];
		//std::cout << m[0][0] << " " << m[1][1] << " " << m[2][2] << " " << m[3][3] << "\n";
	}

	//int parent = item.skeleton.bones[i].parentIndex;

	//for (int i = 0; i < boneCount; i++)
	//{
	//	
	//	int parent = item.skeleton.bones[i].parentIndex;


		//if (parent < 0)
		//	item.globalTransforms[i] = item.localTransforms[i];
		//else
		//	item.globalTransforms[i] = item.globalTransforms[parent] * item.localTransforms[i];
	//}
}

void Animator::computeFinalMatrices(ItemInterface& item)
{
	int boneCount = item.skeleton.bones.size();

	for (int i = 0; i < boneCount; i++)
	{
		item.boneMatrices[i] = item.skeleton.globalInverseTransform * item.globalTransforms[i] *
			item.skeleton.bones[i].inverseBindMatrix;


		//const glm::mat4& m = item.globalTransforms[i];
		//const glm::mat4& m = item.boneMatrices[i];
		//const glm::mat4& m = item.skeleton.bones[i].inverseBindMatrix;
		//std::cout << m[0][0] << " " << m[1][1] << " " << m[2][2] << " " << m[3][3] << "\n";
	}
}

void Animator::update(float deltaTime, ItemInterface& item) {
	float tps = item.animatorData.ticksPerSecond != 0 ? item.animatorData.ticksPerSecond : 25.0f;

	item.currentTimeAnim += deltaTime * tps;
	item.currentTimeAnim = fmod(item.currentTimeAnim, item.animatorData.duration);

	computeLocalTransforms(item);
	computeGlobalTransforms(item);
	computeFinalMatrices(item);
}