#include "System.hpp"
#include "Settings.hpp"
#include "Types.hpp"

#define GLFW_INCLUDE_VULKAN
#define GLFW_DLL
#include <GLFW/glfw3.h>

//!
//!	Finished fixed functions, next is render passes
//!

class TriangleApp {
public:
	void Run() {
		InitWindow();
		InitVulkan();
		MainLoop();
		CleanUp();
	}
private:
	GLFWwindow* window;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchainImages;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExtent;
	std::vector<VkImageView> swapchainImageViews;
	VkPipelineLayout pipelineLayout;

	struct QueueFamilyIndices {
		std::optional<uint> graphicsFamily;
		std::optional<uint> presentFamily;

		bool IsComplete() {
			return graphicsFamily.has_value() and presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	void InitWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(Settings::windowWidth, Settings::windowHeight, Settings::windowTitle.c_str(), nullptr, nullptr);
	}

	void InitVulkan() {
		CreateInstance();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateImageViews();
		CreateGraphicsPipeline();
	}

	void CreateGraphicsPipeline() {
		std::string vertCode = ReadFile("src/Shaders/first.vert.spv");
		std::string fragCode = ReadFile("src/Shaders/first.frag.spv");

		VkShaderModule vertModule = CreateShaderModule(vertCode);
		VkShaderModule fragModule = CreateShaderModule(fragCode);

		VkPipelineShaderStageCreateInfo vertStageInfo {};
		vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertStageInfo.module = vertModule;
		vertStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragStageInfo {};
		fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragStageInfo.module = fragModule;
		fragStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertStageInfo, fragStageInfo};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo {};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = float(swapchainExtent.width);
		viewport.height = float(swapchainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor {};
		scissor.offset = {0, 0};
		scissor.extent = swapchainExtent;

		VkPipelineViewportStateCreateInfo viewportInfo {};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = &viewport;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizerInfo {};
		rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerInfo.depthClampEnable = VK_FALSE;
		rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizerInfo.lineWidth = 1.0f; // Optional
		rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizerInfo.depthBiasEnable = VK_FALSE;
		rasterizerInfo.depthBiasConstantFactor = 0.0f;
		rasterizerInfo.depthBiasClamp = 0.0f;
		rasterizerInfo.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisamplingInfo {};
		multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingInfo.sampleShadingEnable = VK_FALSE;
		multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplingInfo.minSampleShading = 1.0f; // Optional
		multisamplingInfo.pSampleMask = nullptr; // Optional
		multisamplingInfo.alphaToCoverageEnable = VK_FALSE; // Optional
		multisamplingInfo.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineDepthStencilStateCreateInfo depthStencilInfo {};

		VkPipelineColorBlendAttachmentState colorBlendAttachment {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlendInfo {};
		colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendInfo.logicOpEnable = VK_FALSE;
		colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlendInfo.attachmentCount = 1;
		colorBlendInfo.pAttachments = &colorBlendAttachment;
		colorBlendInfo.blendConstants[0] = 0.0f; // Optional
		colorBlendInfo.blendConstants[1] = 0.0f; // Optional
		colorBlendInfo.blendConstants[2] = 0.0f; // Optional
		colorBlendInfo.blendConstants[3] = 0.0f; // Optional

		VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo {};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = 2;
		dynamicStateInfo.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo layoutInfo {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0; // Optional
		layoutInfo.pSetLayouts = nullptr; // Optional
		layoutInfo.pushConstantRangeCount = 0; // Optional
		layoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("Couldn't create a pipeline layout.");
		}


		vkDestroyShaderModule(device, vertModule, nullptr);
		vkDestroyShaderModule(device, fragModule, nullptr);
	}

	VkShaderModule CreateShaderModule(const std::string& code) {
		VkShaderModuleCreateInfo createInfo {};

		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32*>(code.data());
		
		VkShaderModule module;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS) {
			throw std::runtime_error("Couldn't create a shader module.");
		}
		return module;
	}

	std::string ReadFile(const std::string& path) {
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		
		if (!file.is_open()) {
			throw std::runtime_error("Couldn't open the shader.");
		}

		uint fileSize = file.tellg();

		std::string res;
		res.resize(fileSize);

		file.seekg(0);
		file.read(res.data(), fileSize);
		file.close();

		return res;
	}

	void CreateImageViews() {
		swapchainImageViews.resize(swapchainImages.size());

		for (uint i = 0; i < swapchainImageViews.size(); i++) {
			VkImageViewCreateInfo createInfo {};

			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapchainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapchainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create image views.");
			}
		}
	}

	void CreateSwapChain() {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupportDetails(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = BestSwapSurfaceFormatAvailable(swapChainSupport.surfaceFormats);
		VkPresentModeKHR presentMode = BestSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = BestSwapExtent(swapChainSupport.capabilities);

		uint imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount != 0) {
			imageCount = std::min(imageCount, swapChainSupport.capabilities.maxImageCount);
		}

		VkSwapchainCreateInfoKHR createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
		std::vector<uint> queueFamilyIndices = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		if (indices.graphicsFamily == indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a swap chain.");
		}

		imageCount = 0;
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
		swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

		swapchainImageFormat = surfaceFormat.format;
		swapchainExtent = extent;
	}

	VkSurfaceFormatKHR BestSwapSurfaceFormatAvailable(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const VkSurfaceFormatKHR& format : availableFormats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB and format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return format;
			}
		}

		//TODO Maybe add a ranking mechanism for different formats
		return availableFormats[0];
	}

	VkPresentModeKHR BestSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes) {
		for (const auto& presentMode : availableModes) {
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return presentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR; //? This mode is guaranteed to be available
	}

	VkExtent2D BestSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		VkExtent2D actualExtent {Settings::windowWidth, Settings::windowHeight};
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	SwapChainSupportDetails QuerySwapChainSupportDetails(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		details.surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.surfaceFormats.data());

		uint presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());

		return details;
	}

	void CreateSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Couldn't create a window surface.");
		}
	}

	void CreateLogicalDevice() {
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queues;
		std::set<uint> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		for (uint uniqueQueueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = uniqueQueueFamily;
			queueCreateInfo.queueCount = 1;
			float queuePriority = 1.0;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queues.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = uint(queues.size());
		createInfo.pQueueCreateInfos = queues.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = uint(Settings::deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = Settings::deviceExtensions.data();

		if (Settings::useValidationLayers) {
			createInfo.enabledLayerCount = uint(Settings::validationLayers.size());
			createInfo.ppEnabledLayerNames = Settings::validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("Couldn't create a logical device.");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	void PickPhysicalDevice() {
		uint deviceCount = 0;
		
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("Failed to find any GPU with Vulkan support.");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		std::multimap<uint, VkPhysicalDevice> deviceChart;

		for (VkPhysicalDevice& device : devices) {
			uint score = DeviceRating(device);
			if (score != 0) {
				deviceChart.insert(std::make_pair(score, device));
			}
		}

		if (deviceChart.rbegin()->first > 0) {
			physicalDevice = deviceChart.rbegin()->second;
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find suitable GPU.");
		}

		{
			VkPhysicalDeviceProperties deviceProperties;
			uint score = deviceChart.rbegin()->first;
			VkPhysicalDevice device = deviceChart.rbegin()->second;

			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			std::cout << "Chosen physical device:";
			std::cout << "\t" << deviceProperties.deviceName << "\n";
			std::cout << "\tMy score: " << score << "\n";
			std::cout << std::endl;
		}
	}

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (VkQueueFamilyProperties& queueFamilyProperties : queueFamilies) {
			if (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
			VkBool32 presentSuppot = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSuppot);
			if (presentSuppot) {
				indices.presentFamily = i;
			}

			if (indices.IsComplete()) {
				return indices;
			}
			i++;
		}

		return indices;
	}

	bool IsDeviceValid(VkPhysicalDevice device) {
		if (device == VK_NULL_HANDLE) {
			return false;
		}

		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (!DeviceExtensionsSupported(device)) {
			return false;
		}

		QueueFamilyIndices familyIndices = FindQueueFamilies(device);
		if (!familyIndices.IsComplete()) {
			return false;
		}

		SwapChainSupportDetails swapChainDetails = QuerySwapChainSupportDetails(device);
		if (swapChainDetails.surfaceFormats.empty() or swapChainDetails.presentModes.empty()) {
			return false;
		}

		return true;
	}

	bool DeviceExtensionsSupported(VkPhysicalDevice device) {
		uint availableExtensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(Settings::deviceExtensions.begin(), Settings::deviceExtensions.end());

		for (VkExtensionProperties availableExtension : availableExtensions) {
			requiredExtensions.erase(availableExtension.extensionName);
		}

		return requiredExtensions.empty();
	}

	uint DeviceRating(VkPhysicalDevice device) {
		if (!IsDeviceValid(device)) {
			return 0;
		}

		uint score = 1;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		//TODO Add logic to prefer physical devices with drawing and presentation in the same queue for improved performance

		score += deviceProperties.limits.maxImageDimension2D / 10;

		return score;
	}

	void PrintPhysicalDevices() {
		uint deviceCount = 0;
		
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			return;
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		std::cout << "Available physical devices:\n";
		for (VkPhysicalDevice& device : devices) {
			VkPhysicalDeviceProperties deviceProperties;

			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			std::cout << "\t" << deviceProperties.deviceName << "\n";
		}
		std::cout << std::endl;
	}

	void PrintPhysicalDevicesInfo() {
		uint deviceCount = 0;
		
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			return;
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		std::cout << "Physical devices info:\n";
		for (VkPhysicalDevice& device : devices) {
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;

			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
			std::cout << "\nDevice:\n";
			std::cout << "\t" << deviceProperties.deviceName << "\n";
			std::cout << "Properties:\n";
			std::cout << "\tApi version: " << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "." << VK_VERSION_MINOR(deviceProperties.apiVersion) << "." << VK_VERSION_PATCH(deviceProperties.apiVersion) << "\n";
			std::cout << "\tDriver version: " << VK_VERSION_MAJOR(deviceProperties.driverVersion) << "." << VK_VERSION_MINOR(deviceProperties.driverVersion) << "." << VK_VERSION_PATCH(deviceProperties.driverVersion) << "\n";
			std::cout << "\tType: ";
			switch (deviceProperties.deviceType) {
				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
					std::cout << "Integrated GPU";
					break;
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
					std::cout << "Discrete GPU";
					break;
				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
					std::cout << "Virtual GPU";
					break;
				case VK_PHYSICAL_DEVICE_TYPE_CPU:
					std::cout << "CPU";
					break;
				default:
					std::cout << "Other";
			}
			std::cout << "\n";
			std::cout << "Limits:\n";
			std::cout << "\tMax Image Dimension 1D: " << deviceProperties.limits.maxImageDimension1D << "\n";
			std::cout << "\tMax Image Dimension 2D: " << deviceProperties.limits.maxImageDimension2D << "\n";
			std::cout << "\tMax Memory Allocation Count: " << deviceProperties.limits.maxMemoryAllocationCount << "\n";
			std::cout << "\tMax Compute Shared Memory Size: " << deviceProperties.limits.maxComputeSharedMemorySize << "\n";
		}
		std::cout << std::endl;
	}

	void CreateInstance() {
		if (Settings::useValidationLayers and !ValidationLayersPresent()) {
			PrintAvailableValidationLayers();
			throw std::runtime_error("Required validation layers are not present.");
		}

		VkApplicationInfo appInfo {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		if (!InstanceExtensionsPresent()) {
			PrintRequiredInstanceExtensinos();
			PrintAvailableInstanceExtensions();
			throw std::runtime_error("Some instance extensions are unavailable.");
		}

		uint glfwExtensionCount = 0;
		char** glfwExtensions {};
		glfwExtensions = (char**)(glfwGetRequiredInstanceExtensions(&glfwExtensionCount));

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		if (Settings::useValidationLayers)
		{
			createInfo.enabledLayerCount = uint(Settings::validationLayers.size());
			createInfo.ppEnabledLayerNames = Settings::validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}

		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create VkInstance.\nError code: " + std::to_string(int(result)) + ".");
		}
	}

	void PrintAvailableInstanceExtensions() {
		uint extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "Available extensions:\n";
		for (VkExtensionProperties& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
		}
		std::cout << std::endl;
	}

	void PrintRequiredInstanceExtensinos() {
		uint glfwExtensionCount = 0;
		char** glfwExtensions {};
		glfwExtensions = (char**)(glfwGetRequiredInstanceExtensions(&glfwExtensionCount));

		std::cout << "Required extensions:\n";
		for (uint i = 0; i < glfwExtensionCount; i++) {
			std::cout << "\t" << glfwExtensions[i] << "\n";
		}
		std::cout << std::endl;
	}

	bool InstanceExtensionsPresent() {
		uint extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		uint glfwExtensionCount = 0;
		char** glfwExtensions {};
		glfwExtensions = (char**)(glfwGetRequiredInstanceExtensions(&glfwExtensionCount));

		for (uint i = 0; i < glfwExtensionCount; i++) {
			bool extensionPresent = false;

			for (VkExtensionProperties& extension : extensions) {
				if (strcmp(glfwExtensions[i], extension.extensionName) == 0) {
					extensionPresent = true;
					break;
				}
			}

			if (!extensionPresent) {
				return false;
			}
		}

		return true;
	}

	void PrintAvailableValidationLayers() {
		uint layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> layers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

		std::cout << "Available layers:\n";
		for (VkLayerProperties& layer : layers) {
			std::cout << "\t" << layer.layerName << "\n";
		}
		std::cout << std::endl;
	}

	bool ValidationLayersPresent() {
		uint layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> layers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

		for (const char* requiredLayer : Settings::validationLayers) {
			bool layerPresent = false;
			for (VkLayerProperties& availableLayer : layers) {
				if (strcmp(requiredLayer, availableLayer.layerName) == 0) {
					layerPresent = true;
					break;
				}
			}
			if (!layerPresent) {
				return false;
			}
		}

		return true;
	}

	void MainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void CleanUp() {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		for (VkImageView imageView : swapchainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);
		glfwTerminate();
	}
};


int WinMain()
{
	TriangleApp app;

	try {
		app.Run();
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
	return 0;
}