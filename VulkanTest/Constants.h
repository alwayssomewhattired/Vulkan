#pragma once

namespace Constants {

	#ifdef NDEBUG
		inline constexpr bool enableValidationLayers = false;
	#else
		inline constexpr bool enableValidationLayers = true;
	#endif

	static const std::vector<const char*> validationLayers = {
"		VK_LAYER_KHRONOS_validation"
	};

	inline constexpr int MAX_FRAMES_IN_FLIGHT = 2;

};