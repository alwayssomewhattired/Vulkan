#pragma once
#include <array>

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

};