
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

void ModelLoad::modelFileParse(tinygltf::Model& model, const tinygltf::Primitive& primitive, size_t& vertexCount, 
	std::vector<Vertex>& vertices, VkIndexType& indexType, std::vector<unsigned int>& indices, ItemInterface& classReference) {
	// POSITION

	const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
	const auto& posView = model.bufferViews[posAccessor.bufferView];
	const auto& posBuffer = model.buffers[posView.buffer];

	const size_t posOffsetInBuffer = posView.byteOffset + posAccessor.byteOffset;
	const size_t posStride = posView.byteStride ? posView.byteStride : (3 * sizeof(float));
	const unsigned char* base = posBuffer.data.data() + posOffsetInBuffer;
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

	// | materials and textures
	classReference.gpuMaterials().resize(model.materials.size());
	for (int i = 0; i < model.materials.size(); ++i) {
		m_Texture.buildGPUMaterial(model, i, classReference);

	}
}

void ModelLoad::loadModel(const std::string& path, ItemInterface& classReference) {

	auto& vertexBuffer = classReference.vertexBuffer();
	auto& vertexMemory = classReference.vertexMemory();
	auto& indexBuffer = classReference.indexBuffer();
	auto& indexMemory = classReference.indexMemory();
	auto& vertexCount = classReference.vertexCount();
	auto& indexCount = classReference.indexCount();
	auto& indexType = classReference.indexType();
	auto& vertices = classReference.vertices();
	auto& indices = classReference.indices();

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

	//fileDebug(model);

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

	// - this literally gets the first primitive from the mesh and that's it
	// - we need to iterate over all the primitives

	const auto& primitive = mesh.primitives[0];
	modelFileParse(model, primitive, vertexCount, vertices, indexType, indices, classReference);

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

}

// | helpful for printing file properties
void ModelLoad::fileDebug(const tinygltf::Model& model) {

// | images
	std::cout << "Images: " << model.images.size() << "\n";

	for (size_t i = 0; i < model.images.size(); ++i)
	{
		const auto& img = model.images[i];

		std::cout << "Image " << i << "\n";
		std::cout << "  name: " << img.name << "\n";
		std::cout << "  size: " << img.width << " x " << img.height << "\n";
		std::cout << "  components: " << img.component << "\n";
		std::cout << "  bits: " << img.bits << "\n";
		std::cout << "  mimeType: " << img.mimeType << "\n";
		std::cout << "  image data bytes: " << img.image.size() << "\n";
	}

	// | textures
	std::cout << "Textures: " << model.textures.size() << "\n";

	for (size_t i = 0; i < model.textures.size(); ++i)
	{
		const auto& tex = model.textures[i];

		std::cout << "Texture " << i << "\n";
		std::cout << "  source image index: " << tex.source << "\n";
		std::cout << "  sampler index: " << tex.sampler << "\n";
	}

	// PBR
	for (size_t i = 0; i < model.materials.size(); ++i)
	{
		const auto& mat = model.materials[i];
		const auto& pbr = mat.pbrMetallicRoughness;

		std::cout << "Material " << i << "\n";

		if (pbr.baseColorTexture.index >= 0)
			std::cout << "  baseColorTexture: "
			<< pbr.baseColorTexture.index << "\n";

		if (pbr.metallicRoughnessTexture.index >= 0)
			std::cout << "  metallicRoughnessTexture: "
			<< pbr.metallicRoughnessTexture.index << "\n";

		if (mat.normalTexture.index >= 0)
			std::cout << "  normalTexture: "
			<< mat.normalTexture.index << "\n";

		if (mat.emissiveTexture.index >= 0)
			std::cout << "  emissiveTexture: "
			<< mat.emissiveTexture.index << "\n";
	}

}