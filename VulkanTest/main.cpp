
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Initialize.h"

#include "ValidationLayers.h"
#include "Devices.h"
#include "SwapChain.h"
#include "descriptorSetLayout.h"
#include "RenderPass.h"
#include "GraphicsPipeline.h"
#include "CommandPool.h"
#include "AttachmentManager.h"
#include "CommandBuffer.h"
#include "Buffer.h"
#include "Image.h"
#include "Texture.h"
#include "Shaders.h"
#include "Animator.h"
#include "ModelLoad.h"
#include "Camera.h"
#include "Vertex.h"
#include "Constants.h"
#include "UniformBuffer.h"
#include "DescriptorSet.h"
#include "StorageImageManager.h"
#include "RenderTarget.h"
#include "Callbacks.h"
#include "PhysXEngine.h"
#include "items/ItemInterface.h"
#include "items/Triangle.h"
#include "items/Home.h"
#include "items/SilentHill3Game.h"
#include "items/StringLight.h"
#include "items/Table.h"
#include "items/CozyHouse.h"
#include "items/Computer.h"
#include "items/Skeleton.h"


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

const bool enableValidationLayers = Constants::enableValidationLayers;


// | unknown thing that I'm too scared to touch
namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}


// | Camera stuff (necessary to be declared here)
Camera g_Camera{};

// | raw ptr
std::vector<ItemInterface*> items;

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

	std::unique_ptr<Callbacks> m_Callbacks = nullptr;

	std::unique_ptr<Devices> m_devices = nullptr;
	std::unique_ptr<VkDevice> m_device = nullptr;
	std::unique_ptr<VkPhysicalDevice> m_physicalDevice = nullptr;
	std::unique_ptr<VkSampleCountFlagBits> m_msaaSamples = nullptr;
	std::unique_ptr<Devices::QueueFamilyIndices> m_QueueFamilyIndices = nullptr;
	std::unique_ptr<Devices::SwapChainSupportDetails> m_SwapChainSupportDetails = nullptr;
	std::unique_ptr<VkQueue> m_graphicsQueue = nullptr;

	std::unique_ptr<SwapChain> m_SwapChain = nullptr;

	std::unique_ptr<descriptorSetLayout> m_descriptorSetLayout = nullptr;

	std::unique_ptr<Shaders> m_Shaders = nullptr;

	std::unique_ptr<RenderPass> m_RenderPass = nullptr;

	std::unique_ptr<GraphicsPipeline> m_GraphicsPipeline = nullptr;

	std::unique_ptr<CommandPool> m_CommandPool = nullptr;

	std::unique_ptr<AttachmentManager> m_AttachmentManager = nullptr;

	std::unique_ptr<CommandBuffer> m_CommandBuffer = nullptr;

	std::unique_ptr<Buffer> m_Buffer = nullptr;

	std::unique_ptr<Animator> m_Animation = nullptr;

	std::unique_ptr<ModelLoad> m_ModelLoad = nullptr;

	std::unique_ptr<Image> m_Image = nullptr;

	std::unique_ptr<Texture> m_Texture = nullptr;

	std::unique_ptr<Triangle> m_Triangle = nullptr;
	std::unique_ptr<Home> m_Home = nullptr;
	std::unique_ptr<SilentHill3Game> m_SilentHill3Game = nullptr;
	std::unique_ptr<StringLight> m_StringLight = nullptr;
	std::unique_ptr<Table> m_Table = nullptr;
	std::unique_ptr<Computer> m_Computer = nullptr;
	std::unique_ptr<Skeleton> m_Skeleton = nullptr;
	//std::unique_ptr<CozyHouse> m_CozyHouse = nullptr;

	std::unique_ptr<UniformBuffer> m_UniformBuffer = nullptr;

	std::unique_ptr<DescriptorSet> m_DescriptorSet = nullptr;

	std::unique_ptr<StorageImageManager> m_StorageImageManager = nullptr;

	std::unique_ptr<PhysXEngine> m_PhysXEngine = nullptr;

	/// 
	///
	/// 

	/// 
	///
	/// 

	VkSurfaceKHR surface;
	GLFWwindow* window;

	VkInstance instance;

	// - what even is this and why is it here?
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

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

		m_SwapChain = std::make_unique<SwapChain>(*m_devices, surface);

		m_descriptorSetLayout = std::make_unique<descriptorSetLayout>(*m_devices);

		m_Shaders = std::make_unique<Shaders>(*m_device);

		m_RenderPass = std::make_unique<RenderPass>(*m_devices, *m_SwapChain);

		m_GraphicsPipeline = std::make_unique<GraphicsPipeline>(*m_Shaders, *m_device, *m_SwapChain, *m_msaaSamples,
			*m_descriptorSetLayout, m_RenderPass->renderPass);

		m_CommandPool = std::make_unique<CommandPool>();

		m_CommandBuffer = std::make_unique<CommandBuffer>(m_CommandPool->commandPool, *m_devices, *m_SwapChain);
		m_Buffer = std::make_unique<Buffer>(*m_devices, *m_CommandBuffer);
		m_Image = std::make_unique<Image>(*m_devices, *m_CommandBuffer);

		m_CommandPool->createCommandPool(*m_devices);

		m_Texture = std::make_unique<Texture>(*m_Buffer, *m_Image, *m_devices, *m_CommandBuffer);

		m_Animation = std::make_unique<Animator>();

		m_ModelLoad = std::make_unique<ModelLoad>(m_devices->device, m_devices->physicalDevice, m_CommandPool->commandPool,
			m_devices->graphicsQueue, *m_Buffer, *m_CommandBuffer, *m_Texture, *m_Animation);

		m_Triangle = std::make_unique<Triangle>();
		m_Home = std::make_unique<Home>();
		m_SilentHill3Game = std::make_unique<SilentHill3Game>(m_devices->device, m_devices->physicalDevice);
		m_StringLight = std::make_unique<StringLight>(m_devices->device, m_devices->physicalDevice);
		m_Table = std::make_unique<Table>(m_devices->device, m_devices->physicalDevice);
		m_Computer = std::make_unique<Computer>(m_devices->device, m_devices->physicalDevice);
		m_Skeleton = std::make_unique<Skeleton>(m_devices->device, m_devices->physicalDevice);
		//m_CozyHouse = std::make_unique<CozyHouse>(m_devices->device, m_devices->physicalDevice);

		items.push_back(m_Home.get());
		items.push_back(m_SilentHill3Game.get());
		items.push_back(m_StringLight.get());
		items.push_back(m_Table.get());
		items.push_back(m_Computer.get());
		items.push_back(m_Skeleton.get());
		//items.push_back(m_CozyHouse.get());

		m_SwapChain->createSwapChain();
		m_SwapChain->createImageViews(*m_Image);

		m_descriptorSetLayout->createGlobalDescriptorSetLayout();
		m_descriptorSetLayout->createAnimationDescriptorSetLayout();
		m_descriptorSetLayout->createMeshdescriptorSetLayout();
		m_descriptorSetLayout->createMandelbulbComputedescriptorSetLayout();
		m_descriptorSetLayout->createMandelbulbGraphicsdescriptorSetLayout();

		m_AttachmentManager = std::make_unique<AttachmentManager>(*m_SwapChain, *m_msaaSamples, *m_Image, *m_devices);
		m_AttachmentManager->createColorResources();
		m_AttachmentManager->createDepthResources();
		m_RenderPass->createRenderPass(*m_AttachmentManager);

		m_UniformBuffer = std::make_unique<UniformBuffer>(*m_devices, g_Camera, *m_SwapChain, 
			g_renderTarget.rotationEnabled);

		m_UniformBuffer->createUniformBuffers();

		m_PhysXEngine = std::make_unique<PhysXEngine>();

		createModel();

		for (auto& item : items) {
			m_Animation->initialize(*item);
			if (item->hasCollision) {
				item->updatePC();
				m_PhysXEngine->boxCollider(*item);
			}
			m_UniformBuffer->createMaterialUniformBuffer(*item);
			m_UniformBuffer->createAnimationUniformBuffer(*item);
		}

		m_GraphicsPipeline->createGraphicsPipeline(*m_UniformBuffer);

		m_RenderPass->createFramebuffers(m_AttachmentManager->m_GPUColor.view(), m_AttachmentManager->m_GPUDepth.view());


		// | triangle vertex buffer 
		//m_Buffer->createVertexBuffer(m_Triangle->triangleVertices, m_Triangle->vertexBuffer(), m_Triangle->vertexMemory());

	  // - incorporate for triangle in the future
		//createIndexBuffer();

		m_StorageImageManager = std::make_unique<StorageImageManager>(*m_Image, *m_SwapChain, *m_devices);
		m_StorageImageManager->createStorageImageResources();

		m_DescriptorSet = std::make_unique<DescriptorSet>(*m_descriptorSetLayout, *m_devices, *m_UniformBuffer, 
			*m_StorageImageManager, *m_Texture);

		createDescriptorSets();

		m_CommandBuffer->createCommandBuffers(*m_CommandPool);
		m_SwapChain->createSyncObjects();


		std::cout << "Vulkan Engine Initialized\n";
	}

	// | Begin Callbacks block
	// | These need to be in here because
	// | they reference the main class
	// | trust me, do not move them...
	// | Some are static because they have to be

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		HelloTriangleApplication* app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		if (app)
			app->m_Callbacks->onKey(key, scancode, action, mods, window);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	// | This has all the parameters that are allowed
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

		auto& firstMouse = g_Camera.firstMouse;
		auto& lastX = g_Camera.lastX;
		auto& lastY = g_Camera.lastY;

		if (firstMouse) {
			lastX = (float)xpos;
			lastY = (float)ypos;
			firstMouse = false;
		}

		float xoffset = (float)xpos - lastX;
		float yoffset = lastY - (float)ypos; // reversed y

		lastX = (float)xpos;
		lastY = (float)ypos;

		g_Camera.ProcessMouseMovement(xoffset, yoffset);
	}

	// | End Callbacks block


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

		// | This indeed does need to be here this early
		m_Callbacks = std::make_unique<Callbacks>();

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


	// | loads .glb file(s)
	void createModel() {

		m_ModelLoad->loadModel("models/houseofmusic.glb", *m_Home);

		m_ModelLoad->loadModel("models/silent-hill-3-ps2-game-cover/source/SilentHill3ps2Game.glb", *m_SilentHill3Game);

		m_ModelLoad->loadModel("models/lightbulb/scene.gltf", *m_StringLight);

		m_ModelLoad->loadModel("models/Table.glb", *m_Table);

		m_ModelLoad->loadModel("models/computer/source/myComputer.glb", *m_Computer);

		m_ModelLoad->loadModel("models/skeleton/skeleton_animated.FBX", *m_Skeleton);



		// - we crash when loading this.
		// - optimize and check where we are crashing?
		// - this model rules so hard
		//m_ModelLoad->loadModel("models/cozy_house/cozy_houseGLB/cozy_house.glb", * m_CozyHouse);

	}

	// creates descriptor sets for models
	void createDescriptorSets() {


		uint32_t materialsSize = 0;
		for (auto* item : items) {

			materialsSize += item->materialData.gltfMaterials.size();

		}

		m_descriptorSetLayout->createDescriptorPool(materialsSize, items.size());

		m_DescriptorSet->createGlobalDescriptorSets();
		for (auto* item : items) {
			m_DescriptorSet->createMeshDescriptorSets(*item);
			m_DescriptorSet->createAnimationDescriptorSets(*item);
		}

		m_descriptorSetLayout->createComputeDescriptorPool(2);
		m_DescriptorSet->createMandelbulbComputeDescriptorSets();

		m_descriptorSetLayout->createGraphicsDescriptorPool(2);
		m_DescriptorSet->createMandelbulbGraphicsDescriptorSets();
	}


	void drawFrame() {

		vkWaitForFences(*m_device, 1, &m_SwapChain->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;

		if (g_renderTarget.renderMandelbulb) {
			m_UniformBuffer->updateMandelbulbUBO(currentFrame);
		}
		else {
			m_UniformBuffer->updateCameraUniformBuffer(currentFrame);
			for (auto& item : items) {
				item->updatePC();

			}
		}


		// signals 'imageAvailableSemaphores[currentFrame]' when ready
		// acquires next swapchain image (swapchain holds images, not imageviews)
		VkResult result = vkAcquireNextImageKHR(*m_device, m_SwapChain->swapChain, UINT64_MAX, 
			m_SwapChain->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			m_SwapChain->recreateSwapChain(window, *m_Image, *m_AttachmentManager, *m_RenderPass);
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("failed to acquire swap chain image");

		if (m_SwapChain->imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(*m_device, 1, &m_SwapChain->imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		// track swapchain image with current frame's fence
		m_SwapChain->imagesInFlight[imageIndex] = m_SwapChain->inFlightFences[currentFrame];

		vkResetCommandBuffer(m_CommandBuffer->commandBuffers[imageIndex], 0);

		m_CommandBuffer->recordCommandBuffer(m_CommandBuffer->commandBuffers[imageIndex], imageIndex, m_RenderPass->renderPass,
			*m_GraphicsPipeline, items, *m_DescriptorSet, currentFrame, m_StorageImageManager->m_GPUStorageImage.image(), 
			*m_Triangle, *m_UniformBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		// for image ready
		VkSemaphore waitSemaphores[] = { m_SwapChain->imageAvailableSemaphores[currentFrame]};

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer->commandBuffers[imageIndex];

		// for render complete
		VkSemaphore signalSemaphores[] = { m_SwapChain->renderFinishedSemaphores[imageIndex]};

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(*m_device, 1, &m_SwapChain->inFlightFences[currentFrame]);
		if (vkQueueSubmit(*m_graphicsQueue, 1, &submitInfo, m_SwapChain->inFlightFences[currentFrame]) != VK_SUCCESS)
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
			m_SwapChain->recreateSwapChain(window, *m_Image, *m_AttachmentManager, *m_RenderPass);
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
			uint32_t frameIndex = (int)currentFrame % MAX_FRAMES_IN_FLIGHT;

			glfwPollEvents();

			glm::vec3 move(0.0f);

			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				move += g_Camera.GetFront();

			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				move -= g_Camera.GetFront();

			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				move += g_Camera.GetRight();

			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				move -= g_Camera.GetRight();

			m_PhysXEngine->processKeyboard(move);

			// Now after updating camera, we write UBO for current swapchain image
			Camera::CameraUBO ubo{};
			ubo.view = g_Camera.GetViewMatrix();
			ubo.proj = glm::perspective(glm::radians(45.0f), 
				m_SwapChain->swapChainExtent.width / (float)m_SwapChain->swapChainExtent.height, 0.1f, 100.0f);
			ubo.proj[1][1] *= -1.0f;

			// | step physics before render
			m_PhysXEngine->stepPhysics(deltaTime);

			// | update camera with physx
			m_PhysXEngine->readTransforms(g_Camera.Position);
			
			// | animation
			for (auto& item : items) {
				m_Animation->update(deltaTime, *item);
				m_UniformBuffer->updateAnimationUBO(*item, frameIndex); // bones UBO upload
			}

			drawFrame();
		}

		vkDeviceWaitIdle(*m_device);
	}


	void cleanup()
	{ 
		m_SwapChain->cleanupSwapChain(*m_AttachmentManager);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(*m_device, m_UniformBuffer->uniformBuffers[i], nullptr);
			vkFreeMemory(*m_device, m_UniformBuffer->uniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(*m_device, m_descriptorSetLayout->descriptorPool, nullptr);
		vkDestroyDescriptorPool(*m_device, m_descriptorSetLayout->computeDescriptorPool, nullptr);

		vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout->globalDescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout->materialDescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout->mandelbulbComputedescriptorSetLayout, nullptr);
		vkDestroyDescriptorSetLayout(*m_device, m_descriptorSetLayout->mandelbulbGraphicsdescriptorSetLayout, nullptr);

		vkDestroyBuffer(*m_device, indexBuffer, nullptr);
		vkFreeMemory(*m_device, indexBufferMemory, nullptr);

		//vkDestroyBuffer(*m_device, m_Triangle->vertexBuffer()[0], nullptr);
		//vkFreeMemory(*m_device, m_Triangle->vertexMemory()[0], nullptr);

		vkDestroyPipeline(*m_device, m_GraphicsPipeline->graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(*m_device, m_GraphicsPipeline->pipelineLayout, nullptr);
		vkDestroyRenderPass(*m_device, m_RenderPass->renderPass, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(*m_device, m_SwapChain->imageAvailableSemaphores[i], nullptr);
			vkDestroySemaphore(*m_device, m_SwapChain->renderFinishedSemaphores[i], nullptr);
			vkDestroyFence(*m_device, m_SwapChain->inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(*m_device, m_CommandPool->commandPool, nullptr);

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