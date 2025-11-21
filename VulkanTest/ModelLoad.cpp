

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

#include "Vertex.h"



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

		//

		const tinygltf::Mesh& mesh = *meshPtr;

		const auto& primitive = mesh.primitives[0];

		// POSITION
		//
		const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
		const auto& posView = model.bufferViews[posAccessor.bufferView];
		const auto& posBuffer = model.buffers[posView.buffer];

		const size_t posOffsetInBuffer = posView.byteOffset + posAccessor.byteOffset;
		const size_t posStride = posView.byteStride ? posView.byteStride : (3 * sizeof(float));
		const unsigned char* base = posBuffer.data.data() + posOffsetInBuffer;
		std::cout << "posStride: " << posStride << "\n";
		vertexCount = posAccessor.count;
		vertices.resize(vertexCount);

		for (size_t i = 0; i < vertexCount; ++i) {
			const float* p = reinterpret_cast<const float*>(base + i * posStride);
			vertices[i].pos = glm::vec3(p[0], p[1], p[2]);
		}

		// COLOR
		//
		for (size_t i = 0; i < vertexCount; ++i) {
			vertices[i].color = glm::vec3(1.0f); // default color (white)
		}

		// TEXCOORDS
		//
		bool hasTexcoords = primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();

		if (hasTexcoords) {
			const auto& texAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
			
			if (texAccessor.type != TINYGLTF_TYPE_VEC2 || texAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
				throw std::runtime_error("Unsupported TEXCOORD_0 format!");

			const auto& texView = model.bufferViews[texAccessor.bufferView];
			const auto& texBuffer = model.buffers[texView.buffer];

			const  size_t texOffsetInBuffer = texView.byteOffset + texAccessor.byteOffset;
			const size_t texStride = texView.byteStride ? texView.byteStride : (2 * sizeof(float));
			const unsigned char* texBase = texBuffer.data.data() + texOffsetInBuffer;
			size_t texCount = texAccessor.count;

			size_t common = std::min(vertexCount, texCount);
			for (size_t i = 0; i < common; ++i) {
				const float* t = reinterpret_cast<const float*>(texBase + i * texStride);
				vertices[i].texCoord = glm::vec2(t[0], 1.0f - t[1]);
			}

			for (size_t i = common; i < vertexCount; ++i) vertices[i].texCoord = glm::vec2(0.0f);

		}

		// INDICES
		//
		const auto& idxAccessor = model.accessors[primitive.indices];
		const auto& idxView = model.bufferViews[idxAccessor.bufferView];
		const auto& idxBuffer = model.buffers[idxView.buffer];
		
		const size_t idxOffsetInBuffer = idxView.byteOffset + idxAccessor.byteOffset;
		const unsigned char* idxBase = idxBuffer.data.data() + idxOffsetInBuffer;

		indices.resize(idxAccessor.count);

		if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
			const uint16_t* src = reinterpret_cast<const uint16_t*>(idxBase);
			for (size_t i = 0; i < idxAccessor.count; ++i) indices[i] = static_cast<uint32_t>(src[i]);
			indexType = VK_INDEX_TYPE_UINT32; // remember this for vkCmdBindIndexBuffer or vkCmdDrawIndexed
		}
		else if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
			const uint32_t* src = reinterpret_cast<const uint32_t*>(idxBase);
			for (size_t i = 0; i < idxAccessor.count; ++i) indices[i] = src[i];
			indexType = VK_INDEX_TYPE_UINT32;
		}
		else if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
			const uint8_t* src = reinterpret_cast<const uint8_t*>(idxBase);
			for (size_t i = 0; i < idxAccessor.count; ++i) indices[i] = static_cast<uint32_t>(src[i]);
			indexType = VK_INDEX_TYPE_UINT32; // no uint8 in Vulkan so we must expand it to 16 or 32 when uploading
		}
		else
			throw std::runtime_error("Unsupported index component type");


		indexCount = static_cast<uint32_t>(indices.size());

		VkDeviceSize vertexSize = sizeof(Vertex) * vertices.size();

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
		unsigned char* data = reinterpret_cast<unsigned char*>(mapped);
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

		VkDeviceSize indexSize = (indexType == VK_INDEX_TYPE_UINT16 ? sizeof(uint16_t) : sizeof(uint32_t)) * indices.size();

		VkBuffer stagingIb;
		VkDeviceMemory stagingIm;

		createBufferFn(
			indexSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingIb,
			stagingIm
		);

		if (indexType == VK_INDEX_TYPE_UINT16)
		{
			std::vector<uint16_t> tmp(indices.begin(), indices.end());
			vkMapMemory(device, stagingIm, 0, indexSize, 0, &mapped);
			memcpy(mapped, tmp.data(), indexSize);
			vkUnmapMemory(device, stagingIm);
		}
		else {
			vkMapMemory(device, stagingIm, 0, indexSize, 0, &mapped);
			memcpy(mapped, indices.data(), indexSize);
			vkUnmapMemory(device, stagingIm);
		}

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

