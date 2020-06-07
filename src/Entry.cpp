#include "System.hpp"
#include "Settings.hpp"
#include "Types.hpp"

#define GLFW_INCLUDE_VULKAN
#define GLFW_DLL
#include <GLFW/glfw3.h>

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
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	struct QueueFamilyIndices {
		std::optional<uint> graphicsFamily;

		bool IsComplete() {
			return graphicsFamily.has_value();
		}
	};

	void InitWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(Settings::windowWidth, Settings::windowHeight, Settings::windowTitle.c_str(), nullptr, nullptr);
	}

	void InitVulkan() {
		CreateInstance();
		PickPhysicalDevice();
		CreateLogicalDevice();
	}

	void CreateLogicalDevice() {
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		//! I STOPPED HERE
		//!
		//!
		//!
		//!
		//!
		//! HEEEEREEEEE
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

		QueueFamilyIndices familyIndices = FindQueueFamilies(device);
		if (!familyIndices.IsComplete()) {
			return false;
		}

		return true;
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