
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

// | for inverse-bind-matrix and bind-pose
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
void ModelLoad::modelFileParse(const aiScene* scene, aiMesh* mesh, size_t& vertexCount, 
	std::vector<Vertex>& vertices, VkIndexType& indexType, std::vector<uint32_t>& indices,
	ItemInterface& classReference, const uint32_t meshIndex) {

	glm::vec3 min(FLT_MAX);
	glm::vec3 max(-FLT_MAX);

	for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
		vertices[i].pos = {
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		};
		vertices[i].normal = {
			mesh->mNormals[i].x,
			mesh->mNormals[i].y,
			mesh->mNormals[i].z
		};

		if (mesh->mTextureCoords[0]) {
			vertices[i].texCoord = {
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			};
		}

		for (size_t i = 0; i < vertexCount; ++i) {
			vertices[i].color = glm::vec3(1.0f); // default color (white)
		}

		if (mesh->mTangents) {
			vertices[i].tangent = {
				mesh->mTangents[i].x,
				mesh->mTangents[i].y,
				mesh->mTangents[i].z
			};
		}
		else {
			vertices[i].tangent = { 0.0f, 0.0f, 0.0f };
		}

		if (mesh->mBitangents) {
			vertices[i].bitangent = {
				mesh->mBitangents[i].x,
				mesh->mBitangents[i].y,
				mesh->mBitangents[i].z
			};
		}
		else {
			vertices[i].bitangent = { 0.0f, 0.0f, 0.0f };
		}

		min.x = std::min(min.x, vertices[i].pos.x);
		min.y = std::min(min.y, vertices[i].pos.y);
		min.z = std::min(min.z, vertices[i].pos.z);

		max.x = std::max(max.x, vertices[i].pos.x);
		max.y = std::max(max.y, vertices[i].pos.y);
		max.z = std::max(max.z, vertices[i].pos.z);

	}

	glm::vec3 center = (min + max) * 0.5f;
	glm::vec3 extents = (max - min) * 0.5f;

	classReference.center = center;
	classReference.extents = extents;

	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		const aiFace& face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// | bones

	auto& boneMap = classReference.skeleton.boneMap;
	auto& bones = classReference.skeleton.bones;

	for (uint32_t i = 0; i < mesh->mNumBones; i++) {
		aiBone* aiBone = mesh->mBones[i];

		std::string name = aiBone->mName.C_Str();

		int boneIndex;

		if (boneMap.find(name) == boneMap.end()) {
			boneIndex = bones.size();
			boneMap[name] = boneIndex;

			ItemInterface::Bone bone;
			bone.inverseBindMatrix = convert(aiBone->mOffsetMatrix);

			bones.push_back(bone);
		}
		else
		{
			boneIndex = boneMap[name];
		}

		// process vertex weights
		for (uint32_t w = 0; w < aiBone->mNumWeights; w++) {
			aiVertexWeight weight = aiBone->mWeights[w];

			int vertexID = weight.mVertexId;
			float value = weight.mWeight;

			addBoneWeight(vertices[vertexID], boneIndex, value);
		}
	}

	// | Animator
	if (scene->HasAnimations()) {
		aiAnimation* animator = scene->mAnimations[0];

		for (uint32_t i = 0; i < animator->mNumChannels; i++)
		{

			aiNodeAnim* channel = animator->mChannels[i];

			std::string boneName = channel->mNodeName.C_Str();
			int boneIndex = boneMap[boneName];

			Animator::AnimatorChannel animChannel;

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
			// - we crash here
			m_Animator.AnimatorData->channels[boneIndex] = animChannel;
		}

	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	classReference.materialData.gltfPrimitiveMaterialIndices.push_back(mesh->mMaterialIndex);

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

void ModelLoad::processNode(aiNode* node, int parentIndex, ItemInterface& classReference)
{
	auto& boneMap = classReference.skeleton.boneMap;
	auto& bones = classReference.skeleton.bones;

	std::string name = node->mName.C_Str();

	int index = 1;

	if (boneMap.contains(name))
	{
		index = boneMap[name];
		bones[index].parentIndex = parentIndex;
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], index, classReference);
	}
}

void ModelLoad::loadModel(const std::string& path, ItemInterface& classReference) {

	auto& vertexBufferManager = classReference.meshData.vertexBuffer;
	auto& vertexMemoryManager = classReference.meshData.vertexMemory;
	auto& indexBufferManager = classReference.meshData.indexBuffer;
	auto& indexMemoryManager = classReference.meshData.indexMemory;
	auto& vertexCountManager = classReference.meshData.vertexCount;
	auto& indexCountManager = classReference.meshData.indexCount;
	auto& indexType = classReference.meshData.indexType;
	auto& verticesManager = classReference.meshData.vertices;
	auto& indicesManager = classReference.meshData.indices;


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
	classReference.materialData.gltfMaterials.resize(scene->mNumMaterials);

	for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
		
		aiMesh* mesh = scene->mMeshes[i];

		VkBuffer vertexBuffer;
		VkDeviceMemory vertexMemory;
		VkBuffer indexBuffer;
		VkDeviceMemory indexMemory;
		
		std::vector<Vertex> vertices(mesh->mNumVertices);
		std::vector<uint32_t> indices;
		size_t vertexCount = mesh->mNumVertices;
		uint32_t indexCount;

		modelFileParse(scene, mesh, vertexCount, vertices, indexType, indices, classReference, i);
		processNode(scene->mRootNode, -1, classReference);


		// | Vertex 

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

		m_Buffer.createBuffer(
			vertexSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexMemory
		);

		// copy to gpu
		m_CommandBuffer.copyBuffer(stagingVb, vertexBuffer, vertexSize);

		// destroy staging buffer
		vkDestroyBuffer(device, stagingVb, nullptr);
		vkFreeMemory(device, stagingVm, nullptr);


		// indices

		indexType = VK_INDEX_TYPE_UINT32;
		indexCount = static_cast<uint32_t>(indices.size());

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
			indexMemory
		);

		m_CommandBuffer.copyBuffer(stagingIb, indexBuffer, indexSize);

		vkDestroyBuffer(device, stagingIb, nullptr);
		vkFreeMemory(device, stagingIm, nullptr);

		assert(indexType == VK_INDEX_TYPE_UINT32);
		assert(indexSize == sizeof(uint32_t) * indexCount);


		verticesManager.push_back(std::move(vertices));
		indicesManager.push_back(std::move(indices));
		vertexBufferManager.push_back(std::move(vertexBuffer));
		vertexMemoryManager.push_back(std::move(vertexMemory));
		indexBufferManager.push_back(std::move(indexBuffer));
		indexMemoryManager.push_back(std::move(indexMemory));
		vertexCountManager.push_back(std::move(vertexCount));
		indexCountManager.push_back(std::move(indexCount));

	}

}
