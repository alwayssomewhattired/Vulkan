

#include <stb_image.h>
#include <stb_image_write.h>
#include "tinygltf_config.h"
#include <tiny_gltf.h>

#include "ModelLoad.h"
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <functional>
#include <iostream>
#include <string.h>



ModelLoad::ModelLoad(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	VkCommandPool commandPool,
	VkQueue graphicsQueue,
	std::function<void(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&)> createBufferFn,
	std::function<void(VkBuffer, VkBuffer, VkDeviceSize)> copyBufferFn
) 
	: device(device),
	physicalDevice(physicalDevice),
	commandPool(commandPool),
	graphicsQueue(graphicsQueue),
	createBufferFn(createBufferFn),
	copyBufferFn(copyBufferFn)
{}

	void ModelLoad::loadModel(const std::string& path) {

		tinygltf::TinyGLTF loader;
		tinygltf::Model model;
		std::string err, warn;

		bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);

		if (!warn.empty()) std::cout << "Warn: " << warn << "\n";
		if (!err.empty()) std::cout << "Err: " << err << "\n";
		if (!ret) throw std::runtime_error("Failed to load glb file");

		if (model.meshes.empty()) {
			throw std::runtime_error("Model has no meshes!");
		}

		int scene = model.defaultScene;
		const tinygltf::Scene& sceneObj = model.scenes[scene];

		const tinygltf::Mesh* meshPtr = nullptr;

		for (int nodeIndex : sceneObj.nodes) {
			const tinygltf::Node& node = model.nodes[nodeIndex];

			if (node.mesh >= 0) {
				meshPtr = &model.meshes[node.mesh];
			}

		}

		if (!meshPtr)
			throw std::runtime_error("No mesh found in GLB");

		const tinygltf::Mesh& mesh = *meshPtr;

		const auto& primitive = mesh.primitives[0];

		const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
		const auto& posView = model.bufferViews[posAccessor.bufferView];
		const auto& posBuffer = model.buffers[posView.buffer];

		const float* posData = reinterpret_cast<const float*>(&posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);

		vertices.resize(posAccessor.count * 3);
		memcpy(vertices.data(), posData, vertices.size() * sizeof(float));

		const auto& idxAccessor = model.accessors[primitive.indices];
		const auto& idxView = model.bufferViews[idxAccessor.bufferView];
		const auto& idxBuffer = model.buffers[idxView.buffer];

		const uint32_t* idxData = reinterpret_cast<const uint32_t*>(&idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset]);

		indices.resize(idxAccessor.count);
		memcpy(indices.data(), idxData, indices.size() * sizeof(uint32_t));

		indexCount = static_cast<uint32_t>(indices.size());

		VkDeviceSize vertexSize = sizeof(float) * vertices.size();

		VkBuffer stagingVb;
		VkDeviceMemory stagingVm;


		// staging buffer
		createBufferFn(
			vertexSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingVb,
			stagingVm
		);

		void* mapped;
		vkMapMemory(device, stagingVm, 0, vertexSize, 0, &mapped);
		memcpy(mapped, vertices.data(), static_cast<size_t>(vertexSize));
		vkUnmapMemory(device, stagingVm);

		createBufferFn(
			vertexSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexMemory
		);

		// copy to gpu
		copyBufferFn(stagingVb, vertexBuffer, vertexSize);

		// destroy staging buffer
		vkDestroyBuffer(device, stagingVb, nullptr);
		vkFreeMemory(device, stagingVm, nullptr);


		// indices

		VkDeviceSize indexSize = sizeof(uint32_t) * indices.size();

		VkBuffer stagingIb;
		VkDeviceMemory stagingIm;

		createBufferFn(
			indexSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingIb,
			stagingIm
		);

		vkMapMemory(device, stagingIm, 0, indexSize, 0, &mapped);
		memcpy(mapped, indices.data(), static_cast<size_t>(indexSize));
		vkUnmapMemory(device, stagingIm);

		createBufferFn(
			indexSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexMemory
		);

		copyBufferFn(stagingIb, indexBuffer, indexSize);

		vkDestroyBuffer(device, stagingIb, nullptr);
		vkFreeMemory(device, stagingIm, nullptr);
	}

