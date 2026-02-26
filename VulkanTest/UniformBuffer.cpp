#include "UniformBuffer.h"

#include <iostream>




UniformBuffer::UniformBuffer(Devices& devices, Camera& camera, SwapChain& swapChain, const bool& rotationEnabled) : 
	m_Devices(devices), m_Camera(camera), m_SwapChain(swapChain), m_rotationEnabled(rotationEnabled)
{
	// | align dynamic ubo
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(devices.physicalDevice, &props);

	maxPCSize = props.limits.maxPushConstantsSize;

	alignedModelUBOSize =
		(sizeof(ModelUBO) + props.limits.minUniformBufferOffsetAlignment - 1) &
		~(props.limits.minUniformBufferOffsetAlignment - 1);

	modelUBOSize = sizeof(ModelUBO);
}

// - make this generic. don't hardcode our models in here
void UniformBuffer::createUniformBuffers() {

	VkDeviceSize bufferSize = sizeof(Camera::CameraUBO);
	VkDeviceSize modelBufferSize =
		modelUBOSize *
		Constants::MAX_FRAMES_IN_FLIGHT *
		2; // or items.size()
	VkDeviceSize mandelbulbBufferSize = sizeof(MandelbulbUBO);

	uniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	modelUniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	modelUniformBuffersMemory.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	modelUniformBuffersMapped.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	mandelbulbUniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	mandelbulbUniformBuffersMemory.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	mandelbulbUniformBuffersMapped.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	// | triangle path
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

// | creates a specific UniformBuffer
void UniformBuffer::createUniformBuffer(const size_t& UBOSize)
{

	std::vector<VkBuffer> modelUniformBuffers;
	std::vector<VkDeviceMemory> modelUniformBuffersMemory;
	std::vector<void*> modelUniformBuffersMapped;

	//VkDeviceSize modelBufferSize = UBOSize;
	VkDeviceSize modelBufferSize =
		alignedModelUBOSize *
		Constants::MAX_FRAMES_IN_FLIGHT *
		2;
	modelUniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	modelUniformBuffersMemory.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	modelUniformBuffersMapped.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {

		VkBufferCreateInfo modelBufferInfo{};
		modelBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		modelBufferInfo.size = modelBufferSize;
		modelBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		modelBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_Devices.device, &modelBufferInfo, nullptr, &modelUniformBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to create uniform buffer!");

		VkMemoryRequirements modelMemRequirements;
		vkGetBufferMemoryRequirements(m_Devices.device, modelUniformBuffers[i], &modelMemRequirements);

		VkMemoryAllocateInfo modelAllocInfo{};
		modelAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		modelAllocInfo.allocationSize = modelMemRequirements.size;
		modelAllocInfo.memoryTypeIndex = Devices::findMemoryType(modelMemRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Devices.physicalDevice);

		if (vkAllocateMemory(m_Devices.device, &modelAllocInfo, nullptr, &modelUniformBuffersMemory[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate uniform buffer memory");

		vkBindBufferMemory(m_Devices.device, modelUniformBuffers[i], modelUniformBuffersMemory[i], 0);

		vkMapMemory(m_Devices.device, modelUniformBuffersMemory[i], 0, modelBufferSize, 0, &modelUniformBuffersMapped[i]);
	}
}


void UniformBuffer::updateUniformBuffer(uint32_t currentImage) {

	Camera::CameraUBO ubo;
	ubo.view = m_Camera.GetViewMatrix();
	ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChain.swapChainExtent.width / (float)m_SwapChain.swapChainExtent.height, 
		0.1f, 100.0f);
	ubo.proj[1][1] *= -1;

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


void UniformBuffer::updateModelBuffer(uint32_t currentImage) {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	ModelUBO modelUbo{};

	// item view (tilted, rotating)
	if (m_rotationEnabled) {
		modelUbo.model = glm::rotate(glm::mat4(1.0f),
			time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else {
		// normal view (straight, unchanging)
		modelUbo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 4.0f));
	}

	// | scaling
	modelUbo.model = glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 4.0f, 4.0f));

	memcpy(modelUniformBuffersMapped[currentImage], &modelUbo, sizeof(modelUbo));

}