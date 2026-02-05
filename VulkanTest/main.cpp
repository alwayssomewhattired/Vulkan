
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Initialize.h"
#include "MathConfig.hpp"
#include "ValidationLayers.h"
#include "Devices.h"
#include "SwapChain.h"
#include "DescriptorSetLayout.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"
#include "CommandBuffer.h"
#include "Buffer.h"
#include "Image.h"
#include "Texture.h"
#include "Shaders.h"
#include "ModelLoad.h"
#include "Camera.h"
#include "Vertex.h"
#include "Constants.h"
#include "HostToDevice.h"
#include "UniformBuffer.h"
#include "DescriptorSet.h"
#include "items/Home.h"
#include "items/SilentHill3Game.h"


#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>
#include <unordered_map>
#include <cmath>
#include <memory>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = Constants::MAX_FRAMES_IN_FLIGHT;
bool renderTriangle = false;
bool renderMandelbulb = true;

const std::string TEXTURE_PATH = "textures/Metal055C_8K-PNG_Color.png";

const bool enableValidationLayers = Constants::enableValidationLayers;

// | vertices of simple triangle
const std::vector<Vertex> triangleVertices = {
{{ 0.0f, -0.5f, 0.0f }, {1.0f, 0.0f, 0.0f}},
{{ 0.5f,  0.5f, 0.0f }, {0.0f, 1.0f, 0.0f}},
{{-0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 1.0f}}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}


// camera globals
Camera camera;
bool cameraEnabled = true;
double lastX = 0, lastY = 0;
bool firstMouse = true;


class HelloTriangleApplication
{
public:
	void run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:


	std::unique_ptr<ValidationLayers> m_ValidationLayers = nullptr;

	std::unique_ptr<Devices> m_devices = nullptr;
	std::unique_ptr<VkDevice> m_device = nullptr;
	std::unique_ptr<VkPhysicalDevice> m_physicalDevice = nullptr;
	std::unique_ptr<VkSampleCountFlagBits> m_msaaSamples = nullptr;
	std::unique_ptr<Devices::QueueFamilyIndices> m_QueueFamilyIndices = nullptr;
	std::unique_ptr<Devices::SwapChainSupportDetails> m_SwapChainSupportDetails = nullptr;
	std::unique_ptr<VkQueue> m_graphicsQueue = nullptr;

	std::unique_ptr<SwapChain> m_SwapChain = nullptr;
	std::unique_ptr<size_t> m_swapChainImageCount = nullptr;

	std::unique_ptr<DescriptorSetLayout> m_DescriptorSetLayout = nullptr;

	std::unique_ptr<Shaders> m_Shaders = nullptr;

	std::unique_ptr<GraphicsPipeline> m_GraphicsPipeline = nullptr;

	std::unique_ptr<CommandPool> m_CommandPool = nullptr;

	std::unique_ptr<CommandBuffer> m_CommandBuffer = nullptr;

	std::unique_ptr<Buffer> m_Buffer = nullptr;

	std::unique_ptr<ModelLoad> m_ModelLoad = nullptr;

	std::unique_ptr<Image> m_Image = nullptr;

	std::unique_ptr<Texture> m_Texture = nullptr;

	std::unique_ptr<Home> m_home = nullptr;
	std::unique_ptr<SilentHill3Game> m_SilentHill3Game = nullptr;

	std::unique_ptr<UniformBuffer> m_UniformBuffer = nullptr;

	std::unique_ptr<DescriptorSet> m_DescriptorSet = nullptr;

	/// 
	///
	/// 

	/// 
	///
	/// 

	VkSurfaceKHR surface;
	GLFWwindow* window;

	VkInstance instance;

	std::unique_ptr<ModelLoad> model;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkImage storageImage;
	VkDeviceMemory storageImageMemory;
	VkImageView storageImageView;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkSampler mandelbulbSampler;

	bool rotationEnabled = false;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBuffer triangleVertexBuffer;
	VkDeviceMemory triangleVertexBufferMemory;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> imagesInFlight;
	std::vector<VkFence> inFlightFences;

	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass;

	std::vector<VkImageView> swapChainImageViews;

	uint32_t currentFrame = 0;
	bool framebufferResized = false;


	void initVulkan()
	{
		if (enableValidationLayers && !ValidationLayers::checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested. but not available!");
		}

		createInstance();

		m_ValidationLayers = std::make_unique<ValidationLayers>(instance);
		m_ValidationLayers->setupDebugMessenger();
		createSurface();
		m_devices = std::make_unique<Devices>(surface, instance, window);
		m_devices->pickPhysicalDevice();
		m_devices->createLogicalDevice();
		m_device = std::make_unique<VkDevice>(m_devices->device);
		m_physicalDevice = std::make_unique<VkPhysicalDevice>(m_devices->physicalDevice);
		m_devices->findQueueFamilies(*m_physicalDevice);
		m_msaaSamples = std::make_unique<VkSampleCountFlagBits>(m_devices->msaaSamples);
		m_QueueFamilyIndices = std::make_unique<Devices::QueueFamilyIndices>(
			m_devices->m_QueueFamilyIndices
		);
		m_devices->querySwapChainSupport(*m_physicalDevice);
		m_SwapChainSupportDetails = std::make_unique<Devices::SwapChainSupportDetails>(m_devices->m_SwapChainSupportDetails);
		m_graphicsQueue = std::make_unique<VkQueue>(m_devices->graphicsQueue);

		//m_HostToDevice = std::make_unique<HostToDevice>(*m_device, *m_physicalDevice);

		m_SwapChain = std::make_unique<SwapChain>(*m_devices, surface);

		m_DescriptorSetLayout = std::make_unique<DescriptorSetLayout>(m_devices->device);

		m_Shaders = std::make_unique<Shaders>(*m_device);

		m_GraphicsPipeline = std::make_unique<GraphicsPipeline>(*m_Shaders, *m_device, *m_SwapChain, *m_msaaSamples,
			*m_DescriptorSetLayout, renderPass);

		m_CommandPool = std::make_unique<CommandPool>();
		// make command buffer + buffer here
		m_CommandBuffer = std::make_unique<CommandBuffer>(*m_CommandPool, *m_devices);
		m_Buffer = std::make_unique<Buffer>(*m_devices, *m_CommandBuffer);
		m_Image = std::make_unique<Image>(*m_devices, *m_CommandBuffer);
		m_Texture = std::make_unique<Texture>(*m_Buffer, *m_Image, *m_devices, *m_CommandBuffer);

		m_ModelLoad = std::make_unique<ModelLoad>(m_devices->device, m_devices->physicalDevice, m_CommandPool->commandPool,
			m_devices->graphicsQueue, *m_Buffer, *m_CommandBuffer, *m_Texture);

		m_home = std::make_unique<Home>();
		m_SilentHill3Game = std::make_unique<SilentHill3Game>(m_devices->device, m_devices->physicalDevice);

		m_SwapChain->createSwapChain();
		m_swapChainImageCount = std::make_unique<size_t>(m_SwapChain->swapChainImageCount);
		createImageViews();
		createRenderPass();
		m_DescriptorSetLayout->createMeshDescriptorSetLayout();
		m_DescriptorSetLayout->createMandelbulbComputeDescriptorSetLayout();
		m_DescriptorSetLayout->createMandelbulbGraphicsDescriptorSetLayout();

		m_GraphicsPipeline->createGraphicsPipeline();
		m_CommandPool->createCommandPool(*m_devices);
		//createCommandPool();
		createColorResources();//
		createDepthResources();//
		createFramebuffers();//

		createModel();
		// | we create texture image AFTER we load model file
		/*m_Texture->createTextureImage();*/
		// - we don't need to call these anymore, as 'createTextureImage' does this for us.
		//createTextureImageView();
		//createTextureSampler();
		//createModel();

		// for triangle
		m_Buffer->createVertexBuffer(triangleVertices, triangleVertexBuffer, triangleVertexBufferMemory);

	  // incorporate for triangle in the future
		//createIndexBuffer();

		m_UniformBuffer = std::make_unique<UniformBuffer>(*m_devices, camera, m_SwapChain->swapChain, rotationEnabled);
		m_UniformBuffer->createUniformBuffer(m_SilentHill3Game->m_modelUBOSize);

		m_DescriptorSet = std::make_unique<DescriptorSet>(*m_DescriptorSetLayout, *m_devices, *m_UniformBuffer);
		m_DescriptorSet->createDescriptorPool();

		createStorageImageResources();
		createDescriptorSets();
		m_DescriptorSet->createMandelbulbComputeDescriptorSets();
		m_DescriptorSet->createMandelbulbGraphicsDescriptorSets();

		createCommandBuffers();
		createSyncObjects();

		std::cout << "Vulkan Engine Initialized\n";
	}


	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}


	void initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// this gets the primary monitor for fullscreen
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		window = glfwCreateWindow(
			mode->width,
			mode->height,
			"Vulkan",
			monitor,
			nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, keyCallback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void createInstance()
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triagnle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "no Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = ValidationLayers::getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(Constants::validationLayers.size());
			createInfo.ppEnabledLayerNames = Constants::validationLayers.data();

			ValidationLayers::populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}
	}


	// CALLBACKS
	//
	//
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		HelloTriangleApplication* app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		if (app)
			app->onKey(key, scancode, action, mods);
	}

	void onKey(int key, int scancode, int action, int mods) {

		if (key == GLFW_KEY_P && action == GLFW_PRESS) {
			cameraEnabled = !cameraEnabled;

			if (cameraEnabled)
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			else
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else if (key == GLFW_KEY_T && action == GLFW_PRESS) {
			if (renderTriangle)
				renderTriangle = false;
			else
				renderTriangle = true;
		}
		else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
			renderMandelbulb = !renderMandelbulb;
		}
		else if (key == GLFW_KEY_G && action == GLFW_PRESS) {
			rotationEnabled = !rotationEnabled;
		}
	}


	static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

		if (firstMouse) {
			lastX = (float)xpos;
			lastY = (float)ypos;
			firstMouse = false;
		}

		float xoffset = (float)xpos - lastX;
		float yoffset = lastY - (float)ypos; // reversed y

		lastX = (float)xpos;
		lastY = (float)ypos;

		camera.ProcessMouseMovement(xoffset, yoffset);
	}


	void createSurface()
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
			throw std::runtime_error("failed to create window surface!");
	}


	// | loads .glb file
	void createModel() {
		// room will be white until we find a way to pass in external file data.
		model->loadModel("models/thedeathofallionceloved.glb", *m_home);
		// - which texture of from our item class do we use?
		//m_Texture->createTextureImage(false, "textures/Metal055C_8K-PNG_Color.png", m_home->);
		model->loadModel("models/silent-hill-3-ps2-game-cover/source/SilentHill3ps2Game.glb", *m_SilentHill3Game);
	}

	void createDescriptorSets() {
		m_DescriptorSet->createMeshDescriptorSets(*m_home);
		m_DescriptorSet->createMeshDescriptorSets(*m_SilentHill3Game);
	}


	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(*m_physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	VkFormat findDepthFormat() {
		return findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	bool hasStencilComponent(VkFormat format) {
		return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
	}

	// create attachment manager class for this
	void createColorResources() {
		VkFormat colorFormat = m_SwapChain->swapChainImageFormat;

		createImage(m_SwapChain->swapChainExtent.width, m_SwapChain->swapChainExtent.height, 1, *m_msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
		colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	void createDepthResources() {
		VkFormat depthFormat = findDepthFormat();

		createImage(m_SwapChain->swapChainExtent.width, m_SwapChain->swapChainExtent.height, 1, *m_msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
	}

	// | FOR COMPUTE SHADER

	// create storage image manager class for this
	void createStorageImageResources() {
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		createImage(m_SwapChain->swapChainExtent.width, m_SwapChain->swapChainExtent.height, 1, VK_SAMPLE_COUNT_1_BIT, format,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			storageImage, storageImageMemory
		);
		storageImageView = createImageView(storageImage, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}


	void createMandelbulbSampler() {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;

		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.mipLodBias = 0.0f;

		if (vkCreateSampler(*m_device, &samplerInfo, nullptr, &mandelbulbSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create sampler for mandelbulb");
		}

	}


	void createSyncObjects() {

		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(*m_swapChainImageCount);
		imagesInFlight.resize(*m_swapChainImageCount, VK_NULL_HANDLE);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(*m_device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(*m_device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS )
			{
				throw std::runtime_error("failed to create per-frame sync objects!");
			}
		}
		for (size_t i = 0; i < *m_swapChainImageCount; i++) {
			if (vkCreateSemaphore(*m_device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create per-image sync objects!");
			}
		}
	}


	void createCommandBuffers() {

		commandBuffers.resize(*m_swapChainImageCount);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(*m_device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate command buffers");

	}


	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer!");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_SwapChain->swapChainExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_SwapChain->swapChainExtent.width);
		viewport.height = static_cast<float>(m_SwapChain->swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_SwapChain->swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// model
		if (!renderTriangle && !renderMandelbulb) {

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->graphicsPipeline);
			{
				VkBuffer vertexBuffers[] = { m_home->m_vertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(commandBuffer, m_home->m_indexBuffer, 0, m_home->m_indexType);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->pipelineLayout, 0, 1, 
					&descriptorSets[currentFrame], 0, nullptr);

				vkCmdDrawIndexed(commandBuffer, m_home->m_indexCount, 1, 0, 0, 0);
			}

			{
				VkBuffer vertexBuffers[] = { m_SilentHill3Game->m_vertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

				vkCmdBindIndexBuffer(commandBuffer, m_SilentHill3Game->m_indexBuffer, 0, m_SilentHill3Game->m_indexType);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->pipelineLayout, 0, 1,
					&descriptorSets[currentFrame], 0, nullptr);

				vkCmdDrawIndexed(commandBuffer, m_SilentHill3Game->m_indexCount, 1, 0, 0, 0);
			}
		}
		// mandelbulb
		else if (!renderTriangle && renderMandelbulb) {
				//
				// COMPUTE PHASE
				//
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_GraphicsPipeline->mandelbulbComputePipeline);

				vkCmdBindDescriptorSets(
					commandBuffer,
					VK_PIPELINE_BIND_POINT_COMPUTE,
					m_GraphicsPipeline->mandelbulbPipelineComputeLayout,
					0,
					1,
					&mandelbulbComputeDescriptorSets[currentFrame],
					0,
					nullptr
				);

				// Ensure image in GENERAL layout
				VkImageMemoryBarrier barrierToGeneral = {};
				barrierToGeneral.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrierToGeneral.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barrierToGeneral.newLayout = VK_IMAGE_LAYOUT_GENERAL;
				barrierToGeneral.srcAccessMask = 0;
				barrierToGeneral.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				barrierToGeneral.image = storageImage;
				barrierToGeneral.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				barrierToGeneral.subresourceRange.levelCount = 1;
				barrierToGeneral.subresourceRange.layerCount = 1;

				vkCmdPipelineBarrier(
					commandBuffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					0, 0, nullptr, 0, nullptr,
					1, &barrierToGeneral
				);

				// Dispatch
				uint32_t groupX = (m_SwapChain->swapChainExtent.width + 15) / 16;
				uint32_t groupY = (m_SwapChain->swapChainExtent.height + 15) / 16;
				vkCmdDispatch(commandBuffer, groupX, groupY, 1);

				// Make writes visible to fragment shader
				VkImageMemoryBarrier barrierToRead = {};
				barrierToRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrierToRead.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
				barrierToRead.newLayout = VK_IMAGE_LAYOUT_GENERAL;
				//barrierToRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrierToRead.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				barrierToRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				barrierToRead.image = storageImage;
				barrierToRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				barrierToRead.subresourceRange.levelCount = 1;
				barrierToRead.subresourceRange.layerCount = 1;

				vkCmdPipelineBarrier(
					commandBuffer,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0, 0, nullptr, 0, nullptr,
					1, &barrierToRead
				);

				//
				// GRAPHICS PHASE
				//
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->mandelbulbGraphicsPipeline);

				vkCmdBindDescriptorSets(
					commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_GraphicsPipeline->mandelbulbPipelineGraphicsLayout,
					0, 1,
					&mandelbulbGraphicsDescriptorSets[currentFrame],
					0, nullptr
				);

				vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		} 
		// triangle
		else if (renderTriangle && !renderMandelbulb){
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->graphicsPipeline);

			VkBuffer triangleVertexBuffers[] = { triangleVertexBuffer };
			VkDeviceSize triangleOffsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, triangleVertexBuffers, triangleOffsets);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

			vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		}

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer");
	}


	void createFramebuffers() {
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			std::array<VkImageView, 3> attachments = {
				colorImageView,
				depthImageView,
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = m_SwapChain->swapChainExtent.width;
			framebufferInfo.height = m_SwapChain->swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(*m_device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create framebuffer!");
		}
	}



	void createRenderPass() {
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapChain->swapChainImageFormat;
		colorAttachment.samples = *m_msaaSamples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = m_SwapChain->swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = findDepthFormat();
		depthAttachment.samples = *m_msaaSamples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(*m_device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
			throw std::runtime_error("failed to create render pass!");

	}



	void createImageViews() {
		swapChainImageViews.resize(m_SwapChain->swapChainImages.size());

		for (size_t i = 0; i < m_SwapChain->swapChainImages.size(); i++) {
			swapChainImageViews[i] = createImageView(m_SwapChain->swapChainImages[i], m_SwapChain->swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
	}


	void cleanupSwapChain() {
		vkDestroyImage(*m_device, colorImage, nullptr);
		vkDestroyImageView(*m_device, colorImageView, nullptr);
		vkFreeMemory(*m_device, colorImageMemory, nullptr);
		vkDestroyImageView(*m_device, depthImageView, nullptr);
		vkDestroyImage(*m_device, depthImage, nullptr);
		vkFreeMemory(*m_device, depthImageMemory, nullptr);

		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(*m_device, framebuffer, nullptr);
		}

		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(*m_device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(*m_device, m_SwapChain->swapChain, nullptr);
	}

	void recreateSwapChain() {
		int width = 0, height = 0;
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(*m_device);

		cleanupSwapChain();

		m_SwapChain->createSwapChain();
		createImageViews();
		createColorResources();
		createDepthResources();
		createFramebuffers();
		createSyncObjects();
	}



	void drawFrame() {

		vkWaitForFences(*m_device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;

		if (renderMandelbulb) {
			updateMandelbulbUBO(currentFrame);
		}
		else {
			updateUniformBuffer(currentFrame);
			updateModelBuffer(currentFrame);
		}


		// signals 'imageAvailableSemaphores[currentFrame]' when ready
		// acquires next swapchain image (swapchain holds images, not imageviews)
		VkResult result = vkAcquireNextImageKHR(*m_device, m_SwapChain->swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("failed to acquire swap chain image");

		if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(*m_device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		// track swapchain image with current frame's fence
		imagesInFlight[imageIndex] = inFlightFences[currentFrame];

		vkResetCommandBuffer(commandBuffers[imageIndex], 0);
		recordCommandBuffer(commandBuffers[imageIndex], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// for image ready
		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

		// for render complete
		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[imageIndex]};

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(*m_device, 1, &inFlightFences[currentFrame]);
		if (vkQueueSubmit(*m_graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_SwapChain->swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(m_devices->presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain image");

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	}


	// callback inside the class
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
		if (firstMouse)
			lastX = xpos; lastY = ypos; firstMouse = false;

		float xoffset = float(xpos - lastX);
		float yoffset = float(lastY - ypos); // reversed: y ranges top->bottom
		lastX = xpos; lastY = ypos;

		camera.ProcessMouseMovement(xoffset, yoffset);
	}

	void mainLoop()
	{

		while (!glfwWindowShouldClose(window))
		{
			float currentFrame = (float)glfwGetTime();
			static float lastFrame = currentFrame;
			float deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			glfwPollEvents();

			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
			// Now after updating camera, we write UBO for current swapchain image
			Camera::CameraUBO ubo{};
			ubo.view = camera.GetViewMatrix();
			ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChain->swapChainExtent.width / (float)m_SwapChain->swapChainExtent.height, 0.1f, 100.0f);
			ubo.proj[1][1] *= -1.0f;

			drawFrame();
		}

		vkDeviceWaitIdle(*m_device);
	}


	void cleanup()
	{ 
		cleanupSwapChain();

		vkDestroySampler(*m_device, textureSampler, nullptr);

		vkDestroyImageView(*m_device, textureImageView, nullptr);

		vkDestroyImage(*m_device, textureImage, nullptr);
		vkFreeMemory(*m_device, textureImageMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(*m_device, uniformBuffers[i], nullptr);
			vkFreeMemory(*m_device, uniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(*m_device, descriptorPool, nullptr);

		vkDestroyDescriptorSetLayout(*m_device, m_DescriptorSetLayout->descriptorSetLayout, nullptr);

		vkDestroyBuffer(*m_device, indexBuffer, nullptr);
		vkFreeMemory(*m_device, indexBufferMemory, nullptr);

		vkDestroyBuffer(*m_device, triangleVertexBuffer, nullptr);
		vkFreeMemory(*m_device, triangleVertexBufferMemory, nullptr);

		vkDestroyPipeline(*m_device, m_GraphicsPipeline->graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(*m_device, m_GraphicsPipeline->pipelineLayout, nullptr);
		vkDestroyRenderPass(*m_device, renderPass, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(*m_device, imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(*m_device, renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(*m_device, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(*m_device, commandPool, nullptr);

		vkDestroyDevice(*m_device, nullptr);

		if (enableValidationLayers)
			m_ValidationLayers->DestroyDebugUtilsMessengerEXT(instance, m_ValidationLayers->debugMessenger, nullptr);

		vkDestroySurfaceKHR(instance, surface, nullptr);

		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}
};


int main()
{


	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}