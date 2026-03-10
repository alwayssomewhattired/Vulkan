
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
	Texture& texture
) 
	: device(device),
	physicalDevice(physicalDevice),
	commandPool(commandPool),
	graphicsQueue(graphicsQueue),
	m_Buffer(buffer),
	m_CommandBuffer(commandBuffer),
	m_Texture(texture)
{
}


// | Mutates 'vertices' and 'indices' and 'vertexCount' params
void ModelLoad::modelFileParse(const aiScene* scene, aiMesh* mesh, size_t& vertexCount, 
	std::vector<Vertex>& vertices, VkIndexType& indexType, std::vector<uint32_t>& indices,
	ItemInterface& classReference, const uint32_t meshIndex) {

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
	}


	for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
		const aiFace& face = mesh->mFaces[i];
		for (uint32_t j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	classReference.gltfPrimitiveMaterialIndices().push_back(mesh->mMaterialIndex);

	m_Texture.buildGPUMaterial(scene, material, mesh->mMaterialIndex, classReference, meshIndex);

}

void ModelLoad::loadModel(const std::string& path, ItemInterface& classReference) {

	auto& vertexBufferManager = classReference.vertexBuffer();
	auto& vertexMemoryManager = classReference.vertexMemory();
	auto& indexBufferManager = classReference.indexBuffer();
	auto& indexMemoryManager = classReference.indexMemory();
	auto& vertexCountManager = classReference.vertexCount();
	auto& indexCountManager = classReference.indexCount();
	auto& indexType = classReference.indexType();
	auto& verticesManager = classReference.vertices();
	auto& indicesManager = classReference.indices();


	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		path,
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_FlipUVs
	);

	if (!scene || !scene->HasMeshes()) {
		throw std::runtime_error(importer.GetErrorString());
	}

	classReference.gltfMaterials().resize(scene->mNumMaterials);

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
