
#include <tiny_gltf.h>

#include "ModelLoad.h"
#include "Constants.h"

#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <functional>
#include <iostream>
#include <string.h>
#include <utility>

#include "Vertex.h"

// | transports model data from cpu to gpu storage
ModelLoad::ModelLoad(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkCommandPool commandPool,
	VkQueue graphicsQueue,
	Buffer& buffer,
	CommandBuffer& commandBuffer,
	Texture& texture,
	Animator& animator
) 
	: device(device),
	physicalDevice(physicalDevice),
	commandPool(commandPool),
	graphicsQueue(graphicsQueue),
	m_Buffer(buffer),
	m_CommandBuffer(commandBuffer),
	m_Texture(texture),
	m_Animator(animator)
{
}

glm::mat4 ModelLoad::convert(const aiMatrix4x4& m)
{
	return glm::transpose(glm::mat4(
		m.a1, m.a2, m.a3, m.a4,
		m.b1, m.b2, m.b3, m.b4,
		m.c1, m.c2, m.c3, m.c4,
		m.d1, m.d2, m.d3, m.d4
	));
}

glm::vec3 ModelLoad::convert(const aiVector3D& v)
{
	return glm::vec3(v.x, v.y, v.z);
}

glm::quat ModelLoad::convert(const aiQuaternion& q)
{
	return glm::quat(q.w, q.x, q.y, q.z);
}

// | Mutates 'vertices' and 'indices' and 'vertexCount' params
void ModelLoad::modelFileParse(
	const aiScene* scene, aiMesh* mesh, size_t& vertexCount, 
	std::vector<Vertex>& vertices, VkIndexType& indexType, std::vector<uint32_t>& indices,
	ItemInterface& classReference, const uint32_t meshIndex, const uint32_t globalVertexOffset) {

	glm::vec3 min(FLT_MAX);
	glm::vec3 max(-FLT_MAX);
	uint32_t globalVerticesIndex = 0;
	for (uint32_t i = 0; i < mesh->mNumVertices; i++) {

		Vertex vertex{};
		vertex.pos = {
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		};
		vertex.normal = {
			mesh->mNormals[i].x,
			mesh->mNormals[i].y,
			mesh->mNormals[i].z
		};

		if (mesh->mTextureCoords[0]) {
			vertex.texCoord = {
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			};
		}

		for (size_t i = 0; i < vertexCount; ++i) {
			vertex.color = glm::vec3(1.0f); // default color (white)
		}

		if (mesh->mTangents) {
			vertex.tangent = {
				mesh->mTangents[i].x,
				mesh->mTangents[i].y,
				mesh->mTangents[i].z
			};
		}
		else {
			vertex.tangent = { 0.0f, 0.0f, 0.0f };
		}

		if (mesh->mBitangents) {
			vertex.bitangent = {
				mesh->mBitangents[i].x,
				mesh->mBitangents[i].y,
				mesh->mBitangents[i].z
			};
		}
		else {
			vertex.bitangent = { 0.0f, 0.0f, 0.0f };
		}


		min.x = std::min(min.x, vertex.pos.x);
		min.y = std::min(min.y, vertex.pos.y);
		min.z = std::min(min.z, vertex.pos.z);

		max.x = std::max(max.x, vertex.pos.x);
		max.y = std::max(max.y, vertex.pos.y);
		max.z = std::max(max.z, vertex.pos.z);

		vertices.push_back(std::move(vertex));
		globalVerticesIndex++;

	}

	glm::vec3 center = (min + max) * 0.5f;
	glm::vec3 extents = (max - min) * 0.5f;

	classReference.center = center;
	classReference.extents = extents;

	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		const aiFace& face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j] + globalVertexOffset);
		}
	}

	// | bones

	auto& boneMap = m_Animator.globalSkeleton.boneMap;
	auto& bones = m_Animator.globalSkeleton.bones;
	auto& globalInverseTransform = m_Animator.globalSkeleton.globalInverseTransform;
	globalInverseTransform.push_back(glm::inverse(convert(scene->mRootNode->mTransformation)));
	//auto& boneMap = classReference.skeleton.boneMap;
	//auto& bones = classReference.skeleton.bones;
	//auto& globalInverseTransform = classReference.skeleton.globalInverseTransform;
	//globalInverseTransform = glm::inverse(convert(scene->mRootNode->mTransformation));

	static int globalBoneIndex;

	for (uint32_t i = 0; i < mesh->mNumBones; i++) {
		aiBone* aiBone = mesh->mBones[i];
		std::string name = aiBone->mName.C_Str();

		if (boneMap.find(name) == boneMap.end()) {
			globalBoneIndex = bones.size();
			boneMap[name] = globalBoneIndex;
			

			AnimatorStruct::Bone bone;
			bone.inverseBindMatrix = convert(aiBone->mOffsetMatrix);
			bone.name = name;
			bones.push_back(bone);
		}
		else
		{
			globalBoneIndex = boneMap[name];
		}

		// process vertex weights
		for (uint32_t w = 0; w < aiBone->mNumWeights; w++) {
			aiVertexWeight weight = aiBone->mWeights[w];

			int vertexID = weight.mVertexId;
			float value = weight.mWeight;

			addBoneWeight(vertices[vertexID], globalBoneIndex, value);
		}
	}

	// | Animator
	if (scene->HasAnimations()) {
		// | selects animation from list
		aiAnimation* animator = scene->mAnimations[0];
		classReference.animatorData.channels.reserve(animator->mNumChannels);

		for (uint32_t i = 0; i < animator->mNumChannels; i++)
		{

			aiNodeAnim* channel = animator->mChannels[i];

			std::string nodeName = channel->mNodeName.C_Str();

			AnimatorStruct::AnimatorChannel animChannel;

			for (uint32_t k = 0; k < channel->mNumPositionKeys; k++)
			{
				aiVectorKey key = channel->mPositionKeys[k];

				animChannel.positions.push_back({
					(float)key.mTime,
					convert(key.mValue)
					});
			}

			for (uint32_t k = 0; k < channel->mNumRotationKeys; k++)
			{
				aiQuatKey key = channel->mRotationKeys[k];

				animChannel.rotations.push_back({
					(float)key.mTime,
					convert(key.mValue)
					});
			}

			for (uint32_t k = 0; k < channel->mNumScalingKeys; k++)
			{
				aiVectorKey key = channel->mScalingKeys[k];

				animChannel.scales.push_back({
					(float)key.mTime,
					convert(key.mValue)
					});
			}

			classReference.animatorData.channels[nodeName] = animChannel;
		}

		classReference.animatorData.duration = (float)animator->mDuration;
		classReference.animatorData.ticksPerSecond =
			animator->mTicksPerSecond != 0.0
			? (float)animator->mTicksPerSecond
			: 25.0f; // fallback

	}


	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	m_Texture.buildGPUMaterial(scene, material, mesh->mMaterialIndex, classReference, meshIndex);
	
}

void ModelLoad::addBoneWeight(Vertex& v, int boneID, float weight) {

	for (int i = 0; i < 4; i++) {

		if (v.weights[i] == 0.0f) {
			v.boneIDs[i] = boneID;
			v.weights[i] = weight;
			return;
		}
	}
}

// | currently only processes nodes that are bones
// - make this process the ENTIRE node tree
void ModelLoad::processNode(aiNode* node, int parentIndex, ItemInterface& classReference)
{

	auto& boneMap = classReference.skeleton.boneMap;
	auto& bones = classReference.skeleton.bones;

	std::string name = node->mName.C_Str();

	auto it = boneMap.find(name);
	if (it != boneMap.end())
	{
		int index = it->second;


		bones[index].localBindTransform = convert(node->mTransformation);
		bones[index].parentIndex = parentIndex;

		for (uint32_t i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], index, classReference);
		}
		return;
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], parentIndex, classReference);
	}

}

void ModelLoad::loadModel(const std::string& path, ItemInterface& classReference) {

	auto& vertexBufferManager = m_Buffer.globalVertexBuffer;
	VkDeviceMemory vertexMemoryManager = VK_NULL_HANDLE;
	auto& indexBufferManager = m_Buffer.globalIndexBuffer;
	VkDeviceMemory indexMemoryManager = VK_NULL_HANDLE;
	auto& vertexCountManager = classReference.meshData.vertexCount;
	auto& indexCountManager = classReference.meshData.indexCount;
	auto& vertices = m_Buffer.globalVertices;
	auto& indices = m_Buffer.globalIndices;
	VkIndexType indexType = Constants::INDEX_TYPE;

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		path,
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_FlipUVs |
		aiProcess_LimitBoneWeights
	);

	if (!scene || !scene->HasMeshes()) {
		throw std::runtime_error(importer.GetErrorString());
	}
	classReference.materialData.itemMaterials.resize(scene->mNumMaterials);

	for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
		
		aiMesh* mesh = scene->mMeshes[i];

		uint32_t globalVertexOffset = static_cast<uint32_t>(vertices.size());
		uint32_t globalIndexOffset = static_cast<uint32_t>(indices.size());
		
		size_t vertexCount = mesh->mNumVertices;
		vertices.reserve(vertexCount);

		// | zero because we offset indices instead
		classReference.meshData.vertexOffset.push_back(0);

		modelFileParse(scene, mesh, vertexCount, vertices, indexType, indices, classReference, i, globalVertexOffset);
		processNode(scene->mRootNode, -1, classReference);

		classReference.meshData.firstIndex.push_back(globalIndexOffset);
		classReference.meshData.indexCount.push_back(indices.size() - globalIndexOffset);
	
		globalVertexOffset += mesh->mNumVertices;
		globalIndexOffset += indices.size();
	}

	VkDeviceSize vertexSize = sizeof(Vertex) * vertices.size();
	VkBuffer stagingVb;
	VkDeviceMemory stagingVm;

	// staging buffer
	m_Buffer.createBuffer(
		vertexSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingVb,
		stagingVm
	);

	void* mapped;
	vkMapMemory(device, stagingVm, 0, vertexSize, 0, &mapped);
	memcpy(mapped, vertices.data(), static_cast<size_t>(vertexSize));
	unsigned char* data = reinterpret_cast<unsigned char*>(mapped);
	vkUnmapMemory(device, stagingVm);

	auto& vertexBuffer = m_Buffer.globalVertexBuffer;
	auto& indexBuffer = m_Buffer.globalIndexBuffer;

	m_Buffer.createBuffer(
		vertexSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer,
		vertexMemoryManager
	);

	// copy to gpu
	m_CommandBuffer.copyBuffer(stagingVb, vertexBuffer, vertexSize);

	// destroy staging buffer
	vkDestroyBuffer(device, stagingVb, nullptr);
	vkFreeMemory(device, stagingVm, nullptr);

	// indices

	uint32_t indexCount = static_cast<uint32_t>(indices.size());

	VkDeviceSize indexSize = sizeof(uint32_t) * indices.size();

	VkBuffer stagingIb;
	VkDeviceMemory stagingIm;

	m_Buffer.createBuffer(
		indexSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingIb,
		stagingIm
	);

	vkMapMemory(device, stagingIm, 0, indexSize, 0, &mapped);
	memcpy(mapped, indices.data(), indexSize);
	vkUnmapMemory(device, stagingIm);

	m_Buffer.createBuffer(
		indexSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer,
		indexMemoryManager
	);

	m_CommandBuffer.copyBuffer(stagingIb, indexBuffer, indexSize);

	vkDestroyBuffer(device, stagingIb, nullptr);
	vkFreeMemory(device, stagingIm, nullptr);

}
