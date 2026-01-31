
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
#include "Shaders.h"
#include "ModelLoad.h"
#include "Camera.h"
#include "Vertex.h"
#include "Constants.h"
#include "HostToDevice.h"
#include "items/Home.h"
#include "items/SilentHill3Game.h"

#include <stb_image.h>
#include <tiny_gltf.h>

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

// triangle indices
//std::vector<uint32_t> indices;

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

	std::unique_ptr<HostToDevice> m_HostToDevice = nullptr;

	std::unique_ptr<SwapChain> m_SwapChain = nullptr;
	std::unique_ptr<size_t> m_swapChainImageCount = nullptr;

	std::unique_ptr<DescriptorSetLayout> m_DescriptorSetLayout = nullptr;

	std::unique_ptr<Shaders> m_Shaders = nullptr;

	std::unique_ptr<GraphicsPipeline> m_GraphicsPipeline = nullptr;

	std::unique_ptr<Home> m_home = nullptr;
	std::unique_ptr<SilentHill3Game> m_SilentHill3Game = nullptr;

	/// 
	///
	/// 

	struct ModelUBO {
		alignas(16) glm::mat4 model;
	};

	struct MandelbulbUBO {

		glm::mat4 invProjection;
		glm::mat4 invView;

		glm::vec4 camPos_time;       // xyz = cameraPos, w = time
		glm::vec4 resolution_misc;   // xy = resolution, z = power, w = bail

		alignas(16) int maxIter;
		glm::vec3 pad;               // explicit padding
	};

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

	VkSampler textureSampler;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	uint32_t mipLevels;

	VkImageView textureImageView;

	VkDescriptorPool descriptorPool;

	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkDescriptorSet> mandelbulbComputeDescriptorSets;

	VkSampler mandelbulbSampler;

	std::vector<VkDescriptorSet> mandelbulbGraphicsDescriptorSets;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	std::vector<VkBuffer> modelUniformBuffers;
	std::vector<VkDeviceMemory> modelUniformBuffersMemory;
	std::vector<void*> modelUniformBuffersMapped;

	std::vector<VkBuffer> mandelbulbUniformBuffers;
	std::vector<VkDeviceMemory> mandelbulbUniformBuffersMemory;
	std::vector<void*> mandelbulbUniformBuffersMapped;

	bool rotationEnabled = false;

	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkBuffer triangleVertexBuffer;
	VkDeviceMemory triangleVertexBufferMemory;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> imagesInFlight;
	std::vector<VkFence> inFlightFences;

	VkCommandPool commandPool;

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

		m_HostToDevice = std::make_unique<HostToDevice>(*m_device, *m_physicalDevice);

		m_SwapChain = std::make_unique<SwapChain>(*m_devices, surface);

		m_DescriptorSetLayout = std::make_unique<DescriptorSetLayout>(m_devices->device);

		m_Shaders = std::make_unique<Shaders>(*m_device);

		m_GraphicsPipeline = std::make_unique<GraphicsPipeline>(*m_Shaders, *m_device, *m_SwapChain, *m_msaaSamples,
			*m_DescriptorSetLayout, renderPass);

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
		createCommandPool();
		createColorResources();
		createDepthResources();//
		createFramebuffers();//
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		createModel();

		// for triangle
		createVertexBuffer();

	  // incorporate for triangle in the future
		//createIndexBuffer();

		createUniformBuffers();
		createDescriptorPool();

		createStorageImageResources();
		createMeshDescriptorSets();
		createMandelbulbComputeDescriptorSets();
		createMandelbulbGraphicsDescriptorSets();

		createCommandBuffers();
		createSyncObjects();
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

	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(*m_physicalDevice, imageFormat, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
			throw std::runtime_error("texture image format does not support linear blitting!");

		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR
			);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		endSingleTimeCommands(commandBuffer);
	}

	

	// | loads .glb file
	void createModel() {
		
		// | safe-room
		model = std::make_unique<ModelLoad>(
			*m_device, *m_physicalDevice, commandPool, m_devices->graphicsQueue,
			[&](VkDeviceSize size,
				VkBufferUsageFlags usage,
				VkMemoryPropertyFlags properties,
				VkBuffer &buffer,
				VkDeviceMemory& memory) 
			{ createBuffer(size, usage, properties, buffer, memory); }, 
			[&](VkBuffer srcBuffer,
				VkBuffer dstBuffer,
				VkDeviceSize size) 
			{ copyBuffer(srcBuffer, dstBuffer, size); });

		model->loadModel("models/thedeathofallionceloved.glb", *m_home);
		model->loadModel("models/silent-hill-3-ps2-game-cover/source/SilentHill3ps2Game.glb", *m_SilentHill3Game);

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


	

	void createTextureSampler() {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
		samplerInfo.mipLodBias = 0.0f;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(*m_physicalDevice, &properties);
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(*m_device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture sampler!");
	}



	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, 
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = numSamples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(*m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}


		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(*m_device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = m_devices->findMemoryType(memRequirements.memoryTypeBits, properties, *m_physicalDevice);

		if (vkAllocateMemory(*m_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(*m_device, image, imageMemory, 0);
	}


	void createTextureImage() {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels)
			throw std::runtime_error("failed to load texture image!");

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(*m_device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(*m_device, stagingBufferMemory);

		stbi_image_free(pixels);

		createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

		transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			mipLevels);

		copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));


		generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

		vkDestroyBuffer(*m_device, stagingBuffer, nullptr);
		vkFreeMemory(*m_device, stagingBufferMemory, nullptr);
	}


	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(*m_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image view!");
		}

		return imageView;
	}



	void createTextureImageView() {
		textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	}

	
	

	void createDescriptorPool() {
		const uint32_t descriptorCount = 4;

		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = descriptorCount * 3; // camera + model + mandelbulb per set
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = descriptorCount; // one texture per set
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSizes[2].descriptorCount = descriptorCount; // one storage image per compute set

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = descriptorCount * 4; // 4 mesh + 4 compute + 4 graphics = 12

		if (vkCreateDescriptorPool(*m_device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor pool!");
	}



	// GEOMETRY
	void createMeshDescriptorSets() {

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout->descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(*m_device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate descriptor sets!");

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;			
			bufferInfo.range = sizeof(Camera::CameraUBO);

			VkDescriptorBufferInfo modelBufferInfo{};
			modelBufferInfo.buffer = modelUniformBuffers[i];
			modelBufferInfo.offset = 0;			
			modelBufferInfo.range = sizeof(ModelUBO);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = textureImageView;
			imageInfo.sampler = textureSampler;

			std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &modelBufferInfo;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = descriptorSets[i];
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

		}

		m_HostToDevice->createDescriptorSets(m_SilentHill3Game->m_descriptorSets, m_DescriptorSetLayout->descriptorSetLayout, descriptorPool, 
			uniformBuffers, modelUniformBuffers, textureImageView, textureSampler, m_SilentHill3Game->m_modelUBOSize);
	}

	

	void createMandelbulbComputeDescriptorSets() {
		mandelbulbComputeDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout->mandelbulbComputeDescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;            // must support STORAGE_IMAGE + UBO
		allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(*m_device, &allocInfo, mandelbulbComputeDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate compute descriptor sets!");
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			// UBO
			VkDescriptorBufferInfo uboInfo{};
			uboInfo.buffer = mandelbulbUniformBuffers[i];
			uboInfo.offset = 0;
			uboInfo.range = sizeof(MandelbulbUBO);

			// Storage image
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageView = storageImageView; // created with STORAGE_IMAGE usage
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // for storage image writes

			std::array<VkWriteDescriptorSet, 2> writes{};

			// Binding 0: UBO
			writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[0].dstSet = mandelbulbComputeDescriptorSets[i];
			writes[0].dstBinding = 0;
			writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writes[0].descriptorCount = 1;
			writes[0].pBufferInfo = &uboInfo;

			// Binding 1: storage image
			writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[1].dstSet = mandelbulbComputeDescriptorSets[i];
			writes[1].dstBinding = 1;
			writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			writes[1].descriptorCount = 1;
			writes[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(*m_device, writes.size(), writes.data(), 0, nullptr);
		}
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



	void createMandelbulbGraphicsDescriptorSets() {
		mandelbulbGraphicsDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout->mandelbulbGraphicsDescriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(*m_device, &allocInfo, mandelbulbGraphicsDescriptorSets.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate mandelbulb descriptor sets!");

		createMandelbulbSampler();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			// binding 0: sample image (output of computer shader)
			VkDescriptorImageInfo imgInfo{};
			imgInfo.sampler = mandelbulbSampler;
			imgInfo.imageView = storageImageView;
			imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			// binding 1: UBO
			VkDescriptorBufferInfo mandelbulbBufferInfo{};
			mandelbulbBufferInfo.buffer = mandelbulbUniformBuffers[i];
			mandelbulbBufferInfo.offset = 0;
			mandelbulbBufferInfo.range = sizeof(MandelbulbUBO);

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			// sampler
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = mandelbulbGraphicsDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0; // matches mandelbulb shader binding
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imgInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = mandelbulbGraphicsDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1; // matches mandelbulb shader binding
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &mandelbulbBufferInfo;

			vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}


	void createUniformBuffers() {

		VkDeviceSize bufferSize = sizeof(Camera::CameraUBO);
		VkDeviceSize modelBufferSize = sizeof(ModelUBO);
		VkDeviceSize mandelbulbBufferSize = sizeof(MandelbulbUBO);

		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		modelUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		modelUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		modelUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		mandelbulbUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		mandelbulbUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		mandelbulbUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		// | Triangle (i think)
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			VkBufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = bufferSize;
			bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(*m_device, &bufferInfo, nullptr, &uniformBuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create uniform buffer!");

			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(*m_device, uniformBuffers[i], &memRequirements);

			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = m_devices->findMemoryType(memRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, *m_physicalDevice);

			if (vkAllocateMemory(*m_device, &allocInfo, nullptr, &uniformBuffersMemory[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate uniform buffer memory");

			vkBindBufferMemory(*m_device, uniformBuffers[i], uniformBuffersMemory[i], 0);

			vkMapMemory(*m_device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
		}

		// | safe-romm model
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			VkBufferCreateInfo modelBufferInfo{};
			modelBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			modelBufferInfo.size = modelBufferSize;
			modelBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			modelBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(*m_device, &modelBufferInfo, nullptr, &modelUniformBuffers[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create uniform buffer!");

			VkMemoryRequirements modelMemRequirements;
			vkGetBufferMemoryRequirements(*m_device, modelUniformBuffers[i], &modelMemRequirements);

			VkMemoryAllocateInfo modelAllocInfo{};
			modelAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			modelAllocInfo.allocationSize = modelMemRequirements.size;
			modelAllocInfo.memoryTypeIndex = m_devices->findMemoryType(modelMemRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, *m_physicalDevice);

			if (vkAllocateMemory(*m_device, &modelAllocInfo, nullptr, &modelUniformBuffersMemory[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to allocate uniform buffer memory");

			vkBindBufferMemory(*m_device, modelUniformBuffers[i], modelUniformBuffersMemory[i], 0);

			vkMapMemory(*m_device, modelUniformBuffersMemory[i], 0, modelBufferSize, 0, &modelUniformBuffersMapped[i]);
		}

		// | Mandelbulb
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			VkBufferCreateInfo mandelbulbBufferInfo{};
			mandelbulbBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			mandelbulbBufferInfo.size = mandelbulbBufferSize;
			mandelbulbBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			mandelbulbBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(*m_device, &mandelbulbBufferInfo, nullptr, &mandelbulbUniformBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create mandelbulb uniform buffers\n");
			}

			VkMemoryRequirements mandelbulbMemRequirements;
			vkGetBufferMemoryRequirements(*m_device, mandelbulbUniformBuffers[i], &mandelbulbMemRequirements);

			VkMemoryAllocateInfo mandelbulbAllocInfo{};
			mandelbulbAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			mandelbulbAllocInfo.allocationSize = mandelbulbMemRequirements.size;
			mandelbulbAllocInfo.memoryTypeIndex = m_devices->findMemoryType(mandelbulbMemRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, *m_physicalDevice);

			if (vkAllocateMemory(*m_device, &mandelbulbAllocInfo, nullptr, &mandelbulbUniformBuffersMemory[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate mandelbulb uniform buffer memory\n");
			}

			vkBindBufferMemory(*m_device, mandelbulbUniformBuffers[i], mandelbulbUniformBuffersMemory[i], 0);

			vkMapMemory(*m_device, mandelbulbUniformBuffersMemory[i], 0, mandelbulbBufferSize, 0, &mandelbulbUniformBuffersMapped[i]);
		}

		m_HostToDevice->createUniformBuffer(m_SilentHill3Game->m_modelUBOSize);

	}


	void updateUniformBuffer(uint32_t currentImage) {

		Camera::CameraUBO ubo;
		ubo.view = camera.GetViewMatrix();
		ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChain->swapChainExtent.width / (float)m_SwapChain->swapChainExtent.height, 0.1f, 100.0f);
		ubo.proj[1][1] *= -1;

		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}

	void updateMandelbulbUBO(uint32_t currentImage)
	{
		MandelbulbUBO ubo{};

		// 1. Camera matrices
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), m_SwapChain->swapChainExtent.width / (float)m_SwapChain->swapChainExtent.height, 0.1f, 100.0f);
		proj[1][1] *= -1; // Vulkan Y-flip

		// 2. Inverse matrices for fractal shader
		ubo.invView = glm::inverse(view);
		ubo.invProjection = glm::inverse(proj);

		// 3. Camera position (extract from view matrix) + Time (optional animation)
		ubo.camPos_time = glm::vec4(camera.Position, static_cast<float>(glfwGetTime()));

		// 4. Resolution + Power + Bail
		ubo.resolution_misc = glm::vec4(m_SwapChain->swapChainExtent.width, m_SwapChain->swapChainExtent.height, 8.0f, 2.0f);

		// 5. Fractal param
		ubo.maxIter = 6;


		memcpy(mandelbulbUniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}


	void updateModelBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		ModelUBO modelUbo{};

		// item view (tilted, rotating)((bind to key 'p'))
		if (rotationEnabled) {
			modelUbo.model = glm::rotate(glm::mat4(1.0f),
				time * glm::radians(90.0f),
				glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else {
		// normal view (straight, unchanging)
			modelUbo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 4.0f));
		}

		memcpy(modelUniformBuffersMapped[currentImage], &modelUbo, sizeof(modelUbo));

	}



	void createIndexBuffer() {

		// render triangle
		VkDeviceSize bufferSize = sizeof(triangleVertices[0]) * triangleVertices.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(*m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, triangleVertices.data(), (size_t)bufferSize);
		vkUnmapMemory(*m_device, stagingBufferMemory);

		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer, indexBufferMemory);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(*m_device, stagingBuffer, nullptr);
		vkFreeMemory(*m_device, stagingBufferMemory, nullptr);

	}

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(*m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			throw std::runtime_error("failed to create buffer!");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(*m_device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = m_devices->findMemoryType(memRequirements.memoryTypeBits, properties, *m_physicalDevice);

		if (vkAllocateMemory(*m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate buffer memory!");

		vkBindBufferMemory(*m_device, buffer, bufferMemory, 0);
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}



	void createVertexBuffer() {

		// TRIANGLE
		//
		//
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		VkDeviceSize triangleBufferSize = sizeof(triangleVertices[0]) * triangleVertices.size();
		void* data;

		createBuffer(triangleBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);
		// execution stops error

		vkMapMemory(*m_device, stagingBufferMemory, 0, triangleBufferSize, 0, &data);
		memcpy(data, triangleVertices.data(), (size_t)triangleBufferSize);
		vkUnmapMemory(*m_device, stagingBufferMemory);

		createBuffer(triangleBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			triangleVertexBuffer, triangleVertexBufferMemory);

		copyBuffer(stagingBuffer, triangleVertexBuffer, triangleBufferSize);

		vkDestroyBuffer(*m_device, stagingBuffer, nullptr);
		vkFreeMemory(*m_device, stagingBufferMemory, nullptr);
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


	void createCommandPool() {

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_QueueFamilyIndices->graphicsFamily.value();

		if (vkCreateCommandPool(*m_device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
			throw std::runtime_error("failed to create command pool!");
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

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

				vkCmdDrawIndexed(commandBuffer, m_home->m_indexCount, 1, 0, 0, 0);
			}

			{
				VkBuffer vertexBuffers[] = { m_home->m_vertexBuffer };
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





	VkCommandBuffer beginSingleTimeCommands() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(*m_device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(*m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(*m_graphicsQueue);

		vkFreeCommandBuffers(*m_device, commandPool, 1, &commandBuffer);
	}


	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask == 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		endSingleTimeCommands(commandBuffer);
	}


	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleTimeCommands(commandBuffer);
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


	std::cout << "sham\n";
	HelloTriangleApplication app;
	std::cout << "wow\n";

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