
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
#include "RenderPass.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"
#include "AttachmentManager.h"
#include "CommandBuffer.h"
#include "Buffer.h"
#include "Image.h"
#include "Texture.h"
#include "Shaders.h"
#include "ModelLoad.h"
#include "Camera.h"
#include "Vertex.h"
#include "Constants.h"
#include "UniformBuffer.h"
#include "DescriptorSet.h"
#include "StorageImageManager.h"
#include "items/Home.h"
#include "items/SilentHill3Game.h"
#include "RenderTarget.h"


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
#include "items/ItemInterface.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = Constants::MAX_FRAMES_IN_FLIGHT;

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

	std::unique_ptr<RenderPass> m_RenderPass = nullptr;

	std::unique_ptr<GraphicsPipeline> m_GraphicsPipeline = nullptr;

	std::unique_ptr<CommandPool> m_CommandPool = nullptr;

	std::unique_ptr<AttachmentManager> m_AttachmentManager = nullptr;

	std::unique_ptr<CommandBuffer> m_CommandBuffer = nullptr;

	std::unique_ptr<Buffer> m_Buffer = nullptr;

	std::unique_ptr<ModelLoad> m_ModelLoad = nullptr;

	std::unique_ptr<Image> m_Image = nullptr;

	std::unique_ptr<Texture> m_Texture = nullptr;

	std::unique_ptr<Home> m_home = nullptr;
	std::unique_ptr<SilentHill3Game> m_SilentHill3Game = nullptr;

	std::unique_ptr<UniformBuffer> m_UniformBuffer = nullptr;

	std::unique_ptr<DescriptorSet> m_DescriptorSet = nullptr;

	std::unique_ptr<StorageImageManager> m_StorageImageManager = nullptr;

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

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBuffer triangleVertexBuffer;
	VkDeviceMemory triangleVertexBufferMemory;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> imagesInFlight;
	std::vector<VkFence> inFlightFences;

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

		m_RenderPass = std::make_unique<RenderPass>(*m_devices, *m_SwapChain);

		m_GraphicsPipeline = std::make_unique<GraphicsPipeline>(*m_Shaders, *m_device, *m_SwapChain, *m_msaaSamples,
			*m_DescriptorSetLayout, m_RenderPass->renderPass);

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
		m_DescriptorSetLayout->createMeshDescriptorSetLayout();
		m_DescriptorSetLayout->createMandelbulbComputeDescriptorSetLayout();
		m_DescriptorSetLayout->createMandelbulbGraphicsDescriptorSetLayout();

		m_AttachmentManager = std::make_unique<AttachmentManager>(*m_SwapChain, *m_msaaSamples, *m_Image, *m_devices);
		m_AttachmentManager->createColorResources();
		m_AttachmentManager->createDepthResources();
		m_RenderPass->createRenderPass(*m_AttachmentManager);

		m_GraphicsPipeline->createGraphicsPipeline();
		m_CommandPool->createCommandPool(*m_devices);

		m_RenderPass->createFramebuffers(m_AttachmentManager->m_GPUColor.view(), m_AttachmentManager->m_GPUDepth.view());

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

		m_StorageImageManager = std::make_unique<StorageImageManager>(*m_Image, *m_SwapChain, *m_devices);
		m_StorageImageManager->createStorageImageResources();
		createDescriptorSets();
		m_DescriptorSet->createMandelbulbComputeDescriptorSets();
		m_DescriptorSet->createMandelbulbGraphicsDescriptorSets();

		createCommandBuffers();
		createSyncObjects();

		std::cout << "Vulkan Engine Initialized\n";
	}

	// | Callbacks
	// | These need to be in here because
	// | they reference the main class

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		HelloTriangleApplication* app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		if (app)
			app->onKey(key, scancode, action, mods);
	}

	void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
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




	void createImageViews() {
		swapChainImageViews.resize(m_SwapChain->swapChainImages.size());

		for (size_t i = 0; i < m_SwapChain->swapChainImages.size(); i++) {
			swapChainImageViews[i] = createImageView(m_SwapChain->swapChainImages[i], m_SwapChain->swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}
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
			ubo.proj = glm::perspective(glm::radians(45.0f), 
				m_SwapChain->swapChainExtent.width / (float)m_SwapChain->swapChainExtent.height, 0.1f, 100.0f);
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