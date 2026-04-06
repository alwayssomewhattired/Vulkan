#pragma once

#include <tiny_gltf.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>

#include "items/ItemInterface.h"
#include "Vertex.hpp"
#include "Buffer.h"
#include "CommandBuffer.h"
#include "Texture.h"
#include "GPUTexture.h"
#include "Animator.h"
#include "Constants.h"

// | model loader
class ModelLoad
{
public:

	ModelLoad(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,
		Buffer& buffer, CommandBuffer& commandBuffer, Texture& texture, Animator& animator
		);

	// | loads model into assimp 
	// | fills in item class AND cache
	void loadModel(const std::string& path, ItemInterface& classReference, const bool isCache);
	

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;


	Buffer& m_Buffer;
	CommandBuffer& m_CommandBuffer;
	Texture& m_Texture; 
	Animator& m_Animator;

	void modelFileParse(const aiScene* scene, aiMesh* mesh, size_t& vertexCount, 
		std::vector<Vertex>& vertices,
		VkIndexType& indexType, std::vector<uint32_t>& indices, ItemInterface& classReference, const uint32_t meshOffset,
		const uint32_t globalVertexOffset);

	void addBoneWeight(Vertex& v, int boneID, float weight);
	void processNode(aiNode* node, int parentIndex, ItemInterface& classReference);
	void vertexExtractor(aiMesh* mesh, ItemInterface& item, const uint32_t globalVertexOffset);
	void indexExtractor(aiMesh* mesh, const uint32_t globalVertexOffset);

	//void uploadToGPU();

	glm::mat4 convert(const aiMatrix4x4& m);
	glm::vec3 convert(const aiVector3D& v);
	glm::quat convert(const aiQuaternion& q);

	void fileDebug(const tinygltf::Model& model);
};