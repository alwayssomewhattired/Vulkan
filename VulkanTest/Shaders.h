#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <string>
class Shaders
{
public:
	Shaders(VkDevice& m_device);

	std::vector<char> readFile(const std::string& filename);
	VkShaderModule createShaderModule(const std::vector<char>& code);

private:
	VkDevice& m_device;

};

