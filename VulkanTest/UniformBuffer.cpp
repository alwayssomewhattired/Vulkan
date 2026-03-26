#include "UniformBuffer.h"
#include "items/ItemInterface.h"
#include <iostream>




UniformBuffer::UniformBuffer(Devices& devices, Camera& camera, SwapChain& swapChain, const bool& rotationEnabled) : 
	m_Devices(devices), m_Camera(camera), m_SwapChain(swapChain), m_rotationEnabled(rotationEnabled)
{
	// | align dynamic ubo
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(devices.physicalDevice, &props);

	maxPCSize = props.limits.maxPushConstantsSize;
	cameraUBOSize = sizeof(Camera::CameraUBO);
}

// | Camera and Mandelbulb
void UniformBuffer::createUniformBuffers() {

	VkDeviceSize bufferSize = sizeof(Camera::CameraUBO);

	VkDeviceSize mandelbulbBufferSize = sizeof(MandelbulbUBO);

	uniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	mandelbulbUniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	mandelbulbUniformBuffersMemory.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	mandelbulbUniformBuffersMapped.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	// | camera path
	for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = bufferSize;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_Devices.device, &bufferInfo, nullptr, &uniformBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to create uniform buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Devices.device, uniformBuffers[i], &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = m_Devices.findMemoryType(memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Devices.physicalDevice);

		if (vkAllocateMemory(m_Devices.device, &allocInfo, nullptr, &uniformBuffersMemory[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate uniform buffer memory");

		vkBindBufferMemory(m_Devices.device, uniformBuffers[i], uniformBuffersMemory[i], 0);

		vkMapMemory(m_Devices.device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}

	// | Mandelbulb path
	for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {

		VkBufferCreateInfo mandelbulbBufferInfo{};
		mandelbulbBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		mandelbulbBufferInfo.size = mandelbulbBufferSize;
		mandelbulbBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		mandelbulbBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_Devices.device, &mandelbulbBufferInfo, nullptr, &mandelbulbUniformBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create mandelbulb uniform buffers\n");
		}

		VkMemoryRequirements mandelbulbMemRequirements;
		vkGetBufferMemoryRequirements(m_Devices.device, mandelbulbUniformBuffers[i], &mandelbulbMemRequirements);

		VkMemoryAllocateInfo mandelbulbAllocInfo{};
		mandelbulbAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mandelbulbAllocInfo.allocationSize = mandelbulbMemRequirements.size;
		mandelbulbAllocInfo.memoryTypeIndex = m_Devices.findMemoryType(mandelbulbMemRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Devices.physicalDevice);

		if (vkAllocateMemory(m_Devices.device, &mandelbulbAllocInfo, nullptr, &mandelbulbUniformBuffersMemory[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate mandelbulb uniform buffer memory\n");
		}

		vkBindBufferMemory(m_Devices.device, mandelbulbUniformBuffers[i], mandelbulbUniformBuffersMemory[i], 0);

		vkMapMemory(m_Devices.device, mandelbulbUniformBuffersMemory[i], 0, mandelbulbBufferSize, 0, &mandelbulbUniformBuffersMapped[i]);
	}
}

void UniformBuffer::createMaterialUniformBuffer(ItemInterface& item)
{


	size_t uboSize = sizeof(ItemInterface::MaterialUBO);
	size_t numMaterials = item.materialData.itemMaterials.size();

	std::vector<VkBuffer>& uniformBuffers = item.materialData.materialUniformBuffers;
	std::vector<VkDeviceMemory>& uniformBuffersMemory = item.materialData.materialUniformBuffersMemory;
	std::vector<void*>& uniformBuffersMapped = item.materialData.materialUniformBuffersMapped;

	VkDeviceSize bufferSize = uboSize;

	uniformBuffers.resize(numMaterials * Constants::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(numMaterials * Constants::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(numMaterials * Constants::MAX_FRAMES_IN_FLIGHT);

	for (size_t frame = 0; frame < Constants::MAX_FRAMES_IN_FLIGHT; frame++) {
		for (size_t i = 0; i < numMaterials; i++) {

			size_t idx = frame * numMaterials + i;

			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = bufferSize;
			bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(m_Devices.device, &bufferInfo, nullptr, &uniformBuffers[idx]) != VK_SUCCESS)
				throw std::runtime_error("failed to create uniform buffer!");

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(m_Devices.device, uniformBuffers[idx], &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = Devices::findMemoryType(memRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Devices.physicalDevice);

			if (vkAllocateMemory(m_Devices.device, &allocInfo, nullptr, &uniformBuffersMemory[idx]) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate uniform buffer memory");

			vkBindBufferMemory(m_Devices.device, uniformBuffers[idx], uniformBuffersMemory[idx], 0);

			if (vkMapMemory(m_Devices.device, uniformBuffersMemory[idx], 0, bufferSize, 0, &uniformBuffersMapped[idx]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to map material UBO memory");
			}

			ItemInterface::MaterialUBO materialUBO{};
			materialUBO.baseColorFactor = item.materialData.itemMaterials[i].baseColorFactor;
			memcpy(uniformBuffersMapped[idx], &materialUBO, sizeof(ItemInterface::MaterialUBO));

		}

	}
}

void UniformBuffer::createAnimationUniformBuffer(ItemInterface& item)
{

	size_t uboSize = sizeof(glm::mat4) * Constants::MAX_BONES;

	std::vector<VkBuffer>& uniformBuffers = item.animationUniformBuffers;
	std::vector<VkDeviceMemory>& uniformBuffersMemory = item.animationUniformBuffersMemory;
	std::vector<void*>& uniformBuffersMapped = item.animationUniformBuffersMapped;

	VkDeviceSize bufferSize = uboSize;

	uniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	for (size_t frame = 0; frame < Constants::MAX_FRAMES_IN_FLIGHT; frame++) {

		size_t idx = frame;

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = bufferSize;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_Devices.device, &bufferInfo, nullptr, &uniformBuffers[idx]) != VK_SUCCESS)
			throw std::runtime_error("failed to create uniform buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Devices.device, uniformBuffers[idx], &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Devices::findMemoryType(memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Devices.physicalDevice);

		if (vkAllocateMemory(m_Devices.device, &allocInfo, nullptr, &uniformBuffersMemory[idx]) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate uniform buffer memory");

		vkBindBufferMemory(m_Devices.device, uniformBuffers[idx], uniformBuffersMemory[idx], 0);

		if (vkMapMemory(m_Devices.device, uniformBuffersMemory[idx], 0, bufferSize, 0, &uniformBuffersMapped[idx]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to map material UBO memory");
		}
	}
}


void UniformBuffer::updateCameraUniformBuffer(uint32_t currentImage) {

	Camera::CameraUBO ubo;
	ubo.view = m_Camera.GetViewMatrix();
	ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChain.swapChainExtent.width / (float)m_SwapChain.swapChainExtent.height, 
		0.1f, 100.0f);
	ubo.proj[1][1] *= -1;
	ubo.pos = glm::vec4(m_Camera.Position, 1.0f);

	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void UniformBuffer::updateMandelbulbUBO(uint32_t currentImage)
{
	MandelbulbUBO ubo{};

	// 1. Camera matrices
	glm::mat4 view = m_Camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 
		m_SwapChain.swapChainExtent.width / (float)m_SwapChain.swapChainExtent.height, 0.1f, 100.0f);
	proj[1][1] *= -1; // Vulkan Y-flip

	// 2. Inverse matrices for fractal shader
	ubo.invView = glm::inverse(view);
	ubo.invProjection = glm::inverse(proj);

	// 3. Camera position (extract from view matrix) + Time (optional animation)
	ubo.camPos_time = glm::vec4(m_Camera.Position, static_cast<float>(glfwGetTime()));

	// 4. Resolution + Power + Bail
	ubo.resolution_misc = glm::vec4(m_SwapChain.swapChainExtent.width, m_SwapChain.swapChainExtent.height, 8.0f, 2.0f);

	// 5. Fractal param
	ubo.maxIter = 6;


	memcpy(mandelbulbUniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void UniformBuffer::updateAnimationUBO(ItemInterface& item, uint32_t frameIndex)
{

	void* mapped = item.animationUniformBuffersMapped[frameIndex];
	auto& boneMatrices = item.boneMatrices;
	size_t count = std::min(boneMatrices.size(), (size_t)Constants::MAX_BONES);


	// | copy real bone data
	memcpy(mapped, boneMatrices.data(), sizeof(glm::mat4) * count);

	// | fill remaining bone data if empty

	glm::mat4* matrices = reinterpret_cast<glm::mat4*>(mapped);

	for (size_t i = 0; i < Constants::MAX_BONES; i++)
	{
		if (i < count)
			matrices[i] = item.boneMatrices[i];
		else
		{
			matrices[i] = glm::mat4(1.0f);
		}

	}
}