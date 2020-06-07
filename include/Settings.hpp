#include "System.hpp"

namespace Settings
{
	const int windowWidth = 1024;
	const int windowHeight = 768;
	const std::string windowTitle = "Hello Triangle!";

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	#if DEBUG
		const bool useValidationLayers = true;
	#else
		const bool useValidationLayers = false;
	#endif
}