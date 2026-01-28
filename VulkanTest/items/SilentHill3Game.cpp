#include "SilentHill3Game.h"
#include "../Constants.h"
#include "../Devices.h"
#include "../Camera.h"
#include <stdexcept>
#include <functional>
#include <cstdint>

SilentHill3Game::SilentHill3Game(VkDevice& device, VkPhysicalDevice& physicalDevice) : 
	m_device(device), m_physicalDevice(physicalDevice) {}
