#pragma once

#include <PxPhysicsAPI.h>
#include <glm/fwd.hpp>

#include "../glm_config.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>

#include "../Vertex.h"
#include "../Texture.h"
#include "../animation/AnimatorStruct.h"

class Item {
public:
	std::vector<VkBuffer> vertexBuffers;
};

// | blueprint for all items
struct ItemInterface {


	struct MeshData {

		std::vector<size_t> vertexCount;

		std::vector<uint32_t> firstIndex;

		// | DO add to index buffer values
		// | DONT byte offset into vertex buffer because we already offset indices
		std::vector<uint32_t> vertexOffset;

		std::vector<uint32_t> indexCount;
	};

	MeshData meshData;


	struct MaterialData {

		// | item-owned materials
		std::vector<Texture::ItemMaterial> itemMaterials;

		std::string optionalTexturePath;

		// | indexing: frame * materials.size() + materialIdx
		std::vector<VkDescriptorSet> descriptorSets;

		// | contiguous (material index + frame)
		std::vector<VkBuffer> materialUniformBuffers;
		std::vector<VkDeviceMemory> materialUniformBuffersMemory;
		std::vector<void*> materialUniformBuffersMapped;

		// | indices to globalItemMaterials
		std::vector<uint32_t> materialIndices;
	};

	MaterialData materialData;

	// bones
	//

	//
	AnimatorStruct::Skeleton skeleton;

	std::vector<VkDescriptorSet> animationDescriptorSets;

	std::vector<VkBuffer> animationUniformBuffers;
	std::vector<VkDeviceMemory> animationUniformBuffersMemory;
	std::vector<void*> animationUniformBuffersMapped;

	// | for UBO (final bone matrices)
	std::vector<glm::mat4> boneMatrices;

	float currentTimeAnim = 0.0f;

	std::vector<glm::mat4> localTransforms;
	std::vector<glm::mat4> globalTransforms;

	AnimatorStruct::AnimatorData animatorData;

	struct ModelMatrix {
		alignas(16) glm::mat4 model;
	};

	ModelMatrix modelMatrix;

	struct alignas(16) MaterialUBO {
		glm::vec4 baseColorFactor;
	};
	
	virtual void updatePC() = 0;

	bool hasCollision = false;
	physx::PxRigidStatic* collisionBody = nullptr;


	glm::vec3 center;
	glm::vec3 extents;

	virtual ~ItemInterface() = default;
};