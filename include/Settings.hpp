#include "System.hpp"
#include <vulkan/vulkan.h>

namespace Settings
{
	const int windowWidth = 1024;
	const int windowHeight = 768;
	const std::string windowTitle = "Hello Triangle!";

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation",
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	const uint maxFramesInFlight = 2;

	#if DEBUG
		const bool useValidationLayers = true;
	#else
		const bool useValidationLayers = false;
	#endif
}