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

class Item {
public:
	std::vector<VkBuffer> vertexBuffers;
};

// | blueprint for all items
struct ItemInterface {

	struct MeshData {
		std::vector<VkBuffer> vertexBuffer;

		std::vector<VkDeviceMemory> vertexMemory;

		std::vector<VkBuffer> indexBuffer;
		std::vector<VkDeviceMemory> indexMemory;

		std::vector<size_t> vertexCount;
		std::vector<uint32_t> indexCount;

		VkIndexType indexType;

		// | vertices of primitives
		std::vector<std::vector<Vertex>> vertices;

		std::vector<std::vector<uint32_t>> indices;
	};

	MeshData meshData;

	struct MaterialData {
		// | gltf material indices to gltf textures
		std::vector<Texture::GLTFMaterial> gltfMaterials;

		// | primitive-material indices
		std::vector<int> gltfPrimitiveMaterialIndices;

		std::string optionalTexturePath;

		// | indexing: frame * materials.size() + materialIdx
		std::vector<VkDescriptorSet> descriptorSets;

		// | contiguous (material index + frame)
		std::vector<VkBuffer> materialUniformBuffers;
		std::vector<VkDeviceMemory> materialUniformBuffersMemory;
		std::vector<void*> materialUniformBuffersMapped;
	};

	MaterialData materialData;

	// bones

	struct Bone {
		int parentIndex;
		glm::mat4 inverseBindMatrix;
	};

	struct Skeleton {
		std::unordered_map<std::string, int> boneMap;
		std::vector<Bone> bones;
	};

	Skeleton skeleton;

	std::vector<VkDescriptorSet> animationDescriptorSets;

	std::vector<VkBuffer> animationUniformBuffers;
	std::vector<VkDeviceMemory> animationUniformBuffersMemory;
	std::vector<void*> animationUniformBuffersMapped;

	std::vector<glm::mat4> boneMatrices;

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