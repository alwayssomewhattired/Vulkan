#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "Vertex.h"

namespace Constants {

	#ifdef NDEBUG
		inline constexpr bool enableValidationLayers = false;
	#else
		inline constexpr bool enableValidationLayers = true;
	#endif

	inline constexpr std::array<const char*, 1> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	inline constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	inline constexpr uint8_t WHITE_PIXEL[4] = { 255, 255, 255, 255 };
	inline int DEFAULT_WHITE_TEXTURE_INDEX = -1;

	inline constexpr uint8_t BLACK_PIXEL[4] = { 0, 0, 0, 255 };
	inline int DEFAULT_BLACK_TEXTURE_INDEX = -1;

	inline constexpr uint8_t NORMAL_PIXEL[4] = { 128, 128, 255, 255 };
	inline int DEFAULT_NORMAL_TEXTURE_INDEX = -1;

	inline constexpr int MAX_BONES = 100;
	
};