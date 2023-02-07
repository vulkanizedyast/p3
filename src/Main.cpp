/*
 * Copyright (c) 2023 TU Wien, Institute of Visual Computing & Human-Centered Technology.
 */

// Include our framework and the Vulkan headers:
#include "VulkanLaunchpad.h"
#include <vulkan/vulkan.h>

// Include some local helper functions:
#include "VulkanHelpers.h"
#include "Teapot.h"
#include "Camera.h"

// Include functionality from the standard library:
#include <vector>
#include <unordered_map>
#include <limits>

/* ------------------------------------------------ */
// Some more little helpers directly declared here:
// (Definitions of functions below the main function)
/* ------------------------------------------------ */

/*!
 *	This callback function gets invoked by GLFW whenever a GLFW error occured.
 */
void errorCallbackFromGlfw(int error, const char* description);

/*!
 *	Function that is invoked by GLFW to handle key events like key presses or key releases.
 *	If the ESC key has been pressed, the window will be marked that it should close.
 */
void handleGlfwKeyCallback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods);

/*!
 *	Function that can be used to query whether or not currently, i.e. NOW, a certain button
 *  is pressed down, or not.
 *  @param	glfw_key_code	One of the GLFW key codes.
 *                          I.e., use one of the defines that start with GLFW_KEY_*
 *  @return True if the given key is currently pressed down, false otherwise (i.e. released).
 */
bool isKeyDown(int glfw_key_code);

/*!
 *	Determine the Vulkan instance extensions that are required by GLFW and Vulkan Launchpad.
 *	Required extensions from both sources are combined into one single vector (i.e., in
 *	contiguous memory, so that they can easily be passed to:
 *  VkInstanceCreateInfo::enabledExtensionCount and to VkInstanceCreateInfo::ppEnabledExtensionNames.
 *	@return     A std::vector of const char* elements, containing all required instance extensions.
 *	@example    std::vector<const char*> extensions = getRequiredInstanceExtensions();
 *	            VkInstanceCreateInfo create_info    = {};
 *	            create_info.enabledExtensionCount   = extensions.size();
 *	            create_info.ppEnabledExtensionNames = extensions.data();
 */
std::vector<const char*> getRequiredInstanceExtensions();

/*!
 *	Based on the given physical device and the surface, select a queue family which supports both,
 *	graphics and presentation to the given surface. Return the INDEX of an appropriate queue family!
 *	@return		The index of a queue family which supports the required features shall be returned.
 */
uint32_t selectQueueFamilyIndex(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

/* ------------------------------------------------ */
// Main
/* ------------------------------------------------ */

int main(int argc, char** argv)
{
	VKL_LOG(":::::: WELCOME TO VULKAN LAUNCHPAD ::::::");

	// Install a callback function, which gets invoked whenever a GLFW error occurred:
	glfwSetErrorCallback(errorCallbackFromGlfw);

	// Initialize GLFW:
	if (!glfwInit()) {
		VKL_EXIT_WITH_ERROR("Failed to init GLFW");
	}

	/* --------------------------------------------- */
	// Task 1.1: Create a Window with GLFW
	/* --------------------------------------------- */
	constexpr int window_width  = 800;
	constexpr int window_height = 800;
	constexpr bool fullscreen = false;
	constexpr char* window_title = "Tutorial Window";

	// Use a monitor if we'd like to open the window in fullscreen mode:
	GLFWmonitor* monitor = nullptr;
	if (fullscreen) {
		monitor = glfwGetPrimaryMonitor();
	}

	// Set some window settings before creating the window:
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No need to create a graphics context for Vulkan
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// TODO: Get a valid window handle and assign to window:
	GLFWwindow* window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);

	if (!window) {
		VKL_LOG("If your program reaches this point, that means two things:");
		VKL_LOG("1) Project setup was successful. Everything is working fine.");
		VKL_LOG("2) You haven't implemented the first task, which is creating a window with GLFW.");
		VKL_EXIT_WITH_ERROR("No GLFW window created.");
	}
	VKL_LOG("Task 1.1 done.");

	// Set up a key callback via GLFW here to handle keyboard user input:
	glfwSetKeyCallback(window, handleGlfwKeyCallback);

	/* --------------------------------------------- */
	// Task 1.2: Create a Vulkan Instance
	/* --------------------------------------------- */
	VkInstance vk_instance = VK_NULL_HANDLE;

	// Describe some meta data about this application, and define which Vulkan API version is required:
	VkApplicationInfo application_info = {};                     // Zero-initialize every member
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // Set this struct instance's type
	application_info.pEngineName = "Vulkan Launchpad";           // Set some properties...
	application_info.engineVersion = VK_MAKE_API_VERSION(0, 2023, 1, 0);
	application_info.pApplicationName = "An Introduction to Vulkan";
	application_info.applicationVersion = VK_MAKE_API_VERSION(0, 2023, 1, 1);
	application_info.apiVersion = VK_API_VERSION_1_1;            // Your system needs to support this Vulkan API version.

	// We'll require some extensions (e.g., for presenting something on a window surface, and more):
	std::vector<const char*> required_extensions = getRequiredInstanceExtensions();

	// Layers enable additional functionality. We'd like to enable the standard validation layer,
	// so that we get meaningful and descriptive error messages whenever we messed up something:
	if (!hlpIsInstanceLayerSupported("VK_LAYER_KHRONOS_validation")) {
		VKL_EXIT_WITH_ERROR("Validation layer \"VK_LAYER_KHRONOS_validation\" is not supported.");
	}
	VKL_LOG("Validation layer \"VK_LAYER_KHRONOS_validation\" is supported.");
	std::vector<const char*> enabled_layers{ "VK_LAYER_KHRONOS_validation" };

	// Tie everything from above together in an instance of VkInstanceCreateInfo:
	VkInstanceCreateInfo instance_create_info = {}; // Zero-initialize every member
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // Set this struct instance's type
	instance_create_info.pApplicationInfo = &application_info;

	instance_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
	instance_create_info.ppEnabledLayerNames = &enabled_layers[0];
	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
	instance_create_info.ppEnabledExtensionNames = &required_extensions[0];
	// TODO: Hook in required_extensions using VkInstanceCreateInfo::enabledExtensionCount and VkInstanceCreateInfo::ppEnabledExtensionNames!
	// TODO: Hook in enabled_layers using VkInstanceCreateInfo::enabledLayerCount and VkInstanceCreateInfo::ppEnabledLayerNames!

	// TODO: Use vkCreateInstance to create a vulkan instance handle! Assign it to vk_instance!
	VkResult result = VK_ERROR_INITIALIZATION_FAILED;
	result = vkCreateInstance(&instance_create_info, nullptr, &vk_instance);
	VKL_CHECK_VULKAN_RESULT(result);

	if (!vk_instance) {
		VKL_EXIT_WITH_ERROR("No VkInstance created or handle not assigned.");
	}
	VKL_LOG("Task 1.2 done.");

	/* --------------------------------------------- */
	// Task 1.3: Create a Vulkan Window Surface
	/* --------------------------------------------- */
	VkSurfaceKHR vk_surface = VK_NULL_HANDLE;

	// TODO: Use glfwCreateWindowSurface to create a window surface! Assign its handle to vk_surface!
	result = VK_ERROR_INITIALIZATION_FAILED;
	result = glfwCreateWindowSurface(vk_instance, window, NULL, &vk_surface);
	VKL_CHECK_VULKAN_RESULT(result);

	if (!vk_surface) {
		VKL_EXIT_WITH_ERROR("No VkSurfaceKHR created or handle not assigned.");
	}
	VKL_LOG("Task 1.3 done.");

	/* --------------------------------------------- */
	// Task 1.4 Pick a Physical Device
	/* --------------------------------------------- */
	VkPhysicalDevice vk_physical_device = VK_NULL_HANDLE;

	// TODO: Use vkEnumeratePhysicalDevices get all the available physical device handles!
	//       Select one that is suitable using hlpSelectPhysicalDeviceIndex and assign it to vk_physical_device!
	result = VK_ERROR_INITIALIZATION_FAILED;

	uint32_t num_physical_devices;
  	result = vkEnumeratePhysicalDevices(vk_instance, &num_physical_devices, nullptr);
  	VKL_CHECK_VULKAN_RESULT(result);

	std::vector<VkPhysicalDevice> phys_devices;
	phys_devices.resize(num_physical_devices);

	result = vkEnumeratePhysicalDevices(vk_instance, &num_physical_devices, &phys_devices[0]);
	VKL_CHECK_VULKAN_RESULT(result);

	auto idx_phys_device = hlpSelectPhysicalDeviceIndex(&phys_devices[0],
		static_cast<uint32_t>(phys_devices.size()), vk_surface);

	vk_physical_device = phys_devices[idx_phys_device];

	if (!vk_physical_device) {
		VKL_EXIT_WITH_ERROR("No VkPhysicalDevice selected or handle not assigned.");
	}
	VKL_LOG("Task 1.4 done.");

	/* --------------------------------------------- */
	// Task 1.5: Select a Queue Family
	/* --------------------------------------------- */

	// TODO: Find a suitable queue family and assign its index to the following variable:
	//       Hint: Use selectQueueFamilyIndex, but complete its implementation before!
	uint32_t selected_queue_family_index = selectQueueFamilyIndex(vk_physical_device, vk_surface);

	// Sanity check if we have selected a valid queue family index:
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &queue_family_count, nullptr);
	if (selected_queue_family_index >= queue_family_count) {
		VKL_EXIT_WITH_ERROR("Invalid queue family index selected.");
	}
	VKL_LOG("Task 1.5 done.");

	/* --------------------------------------------- */
	// Task 1.6: Create a Logical Device and Get Queue
	/* --------------------------------------------- */
	VkDevice vk_device = VK_NULL_HANDLE;
	VkQueue  vk_queue  = VK_NULL_HANDLE;

	constexpr float queue_priority = 1.0f;

	VkDeviceQueueCreateInfo queue_create_info = {};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = selected_queue_family_index;
	queue_create_info.queueCount = 1;
	queue_create_info.pQueuePriorities = &queue_priority;

	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.pQueueCreateInfos = &queue_create_info;


	std::vector<const char*> enabled_extensions_for_device = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	device_create_info.ppEnabledExtensionNames = &enabled_extensions_for_device[0];
	device_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions_for_device.size());

	// TODO: Create an instance of VkDeviceCreateInfo and use it to create one queue!
	//        - Hook in queue_create_info at the right place!
	//        - Use VkDeviceCreateInfo::enabledExtensionCount and VkDeviceCreateInfo::ppEnabledExtensionNames
	//         to enable the VK_KHR_SWAPCHAIN_EXTENSION_NAME device extension!
	//        - The other parameters are not required (ensure that they are zero-initialized).
	//       Finally, use vkCreateDevice to create the device and assign its handle to vk_device!
	result = VK_ERROR_INITIALIZATION_FAILED;
	result = vkCreateDevice(vk_physical_device, &device_create_info, nullptr, &vk_device);
	VKL_CHECK_VULKAN_RESULT(result);

	if (!vk_device) {
		VKL_EXIT_WITH_ERROR("No VkDevice created or handle not assigned.");
	}

	vkGetDeviceQueue(vk_device, selected_queue_family_index, 0, &vk_queue);

	// TODO: After device creation, use vkGetDeviceQueue to get the one and only created queue!
	//       Assign its handle to vk_queue!

	if (!vk_queue) {
		VKL_EXIT_WITH_ERROR("No VkQueue selected or handle not assigned.");
	}
	VKL_LOG("Task 1.6 done.");

	/* --------------------------------------------- */
	// Task 1.7: Create Swap Chain
	/* --------------------------------------------- */
	VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;

	VkSurfaceCapabilitiesKHR surface_capabilities = hlpGetPhysicalDeviceSurfaceCapabilities(vk_physical_device, vk_surface);

	// Build the swapchain config struct:
	VkSwapchainCreateInfoKHR swapchain_create_info = {};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = vk_surface;
	swapchain_create_info.minImageCount = surface_capabilities.minImageCount;
	swapchain_create_info.imageArrayLayers = 1u;
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_create_info.preTransform = surface_capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	std::vector<decltype(selected_queue_family_index)> vec_queue_family_indices = { selected_queue_family_index };
	swapchain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(vec_queue_family_indices.size());
	swapchain_create_info.pQueueFamilyIndices = &vec_queue_family_indices[0];

	swapchain_create_info.imageFormat = hlpGetSurfaceImageFormat(vk_physical_device, vk_surface).format;
	swapchain_create_info.imageColorSpace = hlpGetSurfaceImageFormat(vk_physical_device, vk_surface).colorSpace;
	swapchain_create_info.imageExtent.height = window_height;
	swapchain_create_info.imageExtent.width = window_width;
	swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	// TODO: Provide values for:
	//        - VkSwapchainCreateInfoKHR::queueFamilyIndexCount
	//        - VkSwapchainCreateInfoKHR::pQueueFamilyIndices
	//        - VkSwapchainCreateInfoKHR::imageFormat
	//        - VkSwapchainCreateInfoKHR::imageColorSpace
	//        - VkSwapchainCreateInfoKHR::imageExtent
	//        - VkSwapchainCreateInfoKHR::presentMode

	// TODO: Create the swapchain using vkCreateSwapchainKHR and assign its handle to vk_swapchain!
	result = VK_ERROR_INITIALIZATION_FAILED;
	result = vkCreateSwapchainKHR(vk_device, &swapchain_create_info, nullptr, &vk_swapchain);
	VKL_CHECK_VULKAN_RESULT(result);

	if (!vk_swapchain) {
		VKL_EXIT_WITH_ERROR("No VkSwapchainKHR created or handle not assigned.");
	}

	// Create a vector of VkImages with enough memory for all the swap chain's images:
	std::vector<VkImage> swap_chain_images(surface_capabilities.minImageCount);
	// TODO: Use vkGetSwapchainImagesKHR to write VkImage handles into swap_chain_images.data()!
	uint32_t image_count = surface_capabilities.minImageCount;

	result = VK_ERROR_INITIALIZATION_FAILED;
	result = vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &image_count, &swap_chain_images[0]);
	VKL_CHECK_VULKAN_RESULT(result);

	if (swap_chain_images.empty()) {
		VKL_EXIT_WITH_ERROR("Swap chain images not retrieved.");
	}

	if(image_count != surface_capabilities.minImageCount)
	{
		VKL_EXIT_WITH_ERROR("image count mismatch");
	}
	VKL_LOG("Task 1.7 done.");

	/* --------------------------------------------- */
	// Task 1.8: Initialize Vulkan Launchpad
	/* --------------------------------------------- */

	// Gather swapchain config as required by the framework:
	VklSwapchainConfig swapchain_config = {};
	swapchain_config.imageExtent = swapchain_create_info.imageExtent;
	swapchain_config.swapchainHandle = vk_swapchain;
	for (VkImage vk_image : swap_chain_images) {
		VklSwapchainFramebufferComposition framebufferData;
		// TODO: Fill the data for the color attachment:
		//  - VklSwapchainImageDetails::imageHandle
		//  - VklSwapchainImageDetails::imageFormat
		//  - VklSwapchainImageDetails::imageUsage
		//  - VklSwapchainImageDetails::clearValue
		framebufferData.colorAttachmentImageDetails.imageFormat = hlpGetSurfaceImageFormat(vk_physical_device, vk_surface).format;
		framebufferData.colorAttachmentImageDetails.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		framebufferData.colorAttachmentImageDetails.imageHandle = vk_image;
		framebufferData.colorAttachmentImageDetails.clearValue = VkClearValue {
			VkClearColorValue{ 0.39f, 0.58f, 0.93f, 1.0f }
		};

		// We don't need the depth attachment now, but keep it in mind for later!
		framebufferData.depthAttachmentImageDetails.imageHandle = VK_NULL_HANDLE;

		// Add it to the vector:
		swapchain_config.swapchainImages.push_back(framebufferData);
	}

	// Init the framework:
	if (!vklInitFramework(vk_instance, vk_surface, vk_physical_device, vk_device, vk_queue, swapchain_config)) {
		VKL_EXIT_WITH_ERROR("Failed to init Vulkan Launchpad");
	}
	VKL_LOG("Task 1.8 done.");

	VklGraphicsPipelineConfig pipeline_config;
	pipeline_config.vertexShaderPath = "../../shaders/vertex.shader";
	pipeline_config.fragmentShaderPath = "../../shaders/fragment.shader";

	VkVertexInputBindingDescription vertex_input = {};
	vertex_input.binding = 0;
	vertex_input.stride = sizeof(float) * 3;
	vertex_input.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	pipeline_config.vertexInputBuffers.push_back(vertex_input);

	VkVertexInputAttributeDescription vertex_attribute = {};
	vertex_attribute.binding = 0;
	vertex_attribute.location = 0;
	vertex_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;

	pipeline_config.inputAttributeDescriptions.push_back(vertex_attribute);
	pipeline_config.polygonDrawMode = VK_POLYGON_MODE_FILL;
	pipeline_config.triangleCullingMode = VK_CULL_MODE_NONE;

	struct UniformBufferData {
    	glm::vec4 color = glm::vec4{1.0f, 0.25f, 0.0f, 1.0f};
		glm::mat4 transformation = glm::mat4{ 1,  0,  0,  0,
           0, -1,  0,  0,
           0,  0, -1,  0,
           0,  0,  0,  1};
	} uniform_buffer_data;

	VkDescriptorSetLayoutBinding descriptor_binding;
	descriptor_binding.binding = 0;
	descriptor_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_binding.descriptorCount = 1;
	descriptor_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
	descriptor_binding.pImmutableSamplers = nullptr;

	pipeline_config.descriptorLayout.push_back(descriptor_binding);

	auto vk_pipeline = vklCreateGraphicsPipeline(pipeline_config);

	auto vk_uniform_buffer = vklCreateHostCoherentBufferWithBackingMemory(sizeof(uniform_buffer_data),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

	vklCopyDataIntoHostCoherentBuffer(vk_uniform_buffer, &uniform_buffer_data, sizeof(uniform_buffer_data));

	VkDescriptorPoolSize uniform_buffer_pool_size = {};
	uniform_buffer_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniform_buffer_pool_size.descriptorCount = 16;

	VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
	descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_create_info.maxSets = 8;
	descriptor_pool_create_info.poolSizeCount = 1;
	descriptor_pool_create_info.pPoolSizes = &uniform_buffer_pool_size;

	VkDescriptorPool vk_descriptor_pool = nullptr;
	result = vkCreateDescriptorPool(vk_device, &descriptor_pool_create_info, nullptr, &vk_descriptor_pool);
	VKL_CHECK_VULKAN_RESULT(result);

	VkDescriptorSetLayoutCreateInfo descript_set_create_info = {};
	descript_set_create_info.bindingCount = 1;
	descript_set_create_info.pBindings = &descriptor_binding;
	descript_set_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

	VkDescriptorSetLayout vk_descriptor_set_layout = {};
	result = vkCreateDescriptorSetLayout(vk_device, &descript_set_create_info, nullptr, &vk_descriptor_set_layout);
	VKL_CHECK_VULKAN_RESULT(result);

	VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};
	descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptor_set_alloc_info.descriptorPool = vk_descriptor_pool;
	descriptor_set_alloc_info.descriptorSetCount = 1;
	descriptor_set_alloc_info.pSetLayouts = &vk_descriptor_set_layout;

	VkDescriptorSet vk_descriptor_set = nullptr;
	result = vkAllocateDescriptorSets(vk_device, &descriptor_set_alloc_info, &vk_descriptor_set);

	VkDescriptorBufferInfo descriptor_buffer_info = {};
	descriptor_buffer_info.buffer = vk_uniform_buffer;
	descriptor_buffer_info.offset = 0;
	descriptor_buffer_info.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet write_descriptor_set = {};
	write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write_descriptor_set.dstSet = vk_descriptor_set;
	write_descriptor_set.dstBinding = 0;
	write_descriptor_set.descriptorCount = 1;
	write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write_descriptor_set.pBufferInfo = &descriptor_buffer_info;

	vkUpdateDescriptorSets(vk_device, 1, &write_descriptor_set, 0, nullptr);

	VklCameraHandle camera = vklCreateCamera(window);

	teapotCreateGeometryAndBuffers();

	/* --------------------------------------------- */
	// Task 1.9:  Implement the Render Loop
	/* --------------------------------------------- */
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents(); // Handle user input
		vklUpdateCamera(camera);
		glm::mat4 matrix = vklGetCameraViewProjectionMatrix(camera);
		uniform_buffer_data.transformation = matrix;
		vklCopyDataIntoHostCoherentBuffer(vk_uniform_buffer, &uniform_buffer_data, sizeof(uniform_buffer_data));

		vklWaitForNextSwapchainImage();
		vklStartRecordingCommands();
		teapotDraw(vk_pipeline, vk_descriptor_set);
		vklEndRecordingCommands();
		vklPresentCurrentSwapchainImage();
	}

	// Wait for all GPU work to finish before cleaning up:
	vkDeviceWaitIdle(vk_device);

	/* --------------------------------------------- */
	// Task 1.10: Cleanup
	/* --------------------------------------------- */

	vklDestroyCamera(camera);
	vklDestroyHostCoherentBufferAndItsBackingMemory(vk_uniform_buffer);
	vklDestroyGraphicsPipeline(vk_pipeline);

	teapotDestroyBuffers();
	vklDestroyFramework();
	glfwTerminate();

	return EXIT_SUCCESS;
}

/* ------------------------------------------------ */
// Definitions of little helpers defined above main:
/* ------------------------------------------------ */

void errorCallbackFromGlfw(int error, const char* description) {
	std::cout << "GLFW error " << error << ": " << description << std::endl;
}

std::unordered_map<int, bool> g_isGlfwKeyDown;

void handleGlfwKeyCallback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		g_isGlfwKeyDown[key] = true;
	}

	if (action == GLFW_RELEASE) {
		g_isGlfwKeyDown[key] = false;
	}

	// We mark the window that it should close if ESC is pressed:
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(glfw_window, true);
	}
}

bool isKeyDown(int glfw_key_code)
{
	return g_isGlfwKeyDown[glfw_key_code];
}

std::vector<const char*> getRequiredInstanceExtensions()
{
	// Get extensions which GLFW requires:
	uint32_t num_glfw_extensions;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&num_glfw_extensions);

	// Get extensions which Vulkan Launchpad requires:
	uint32_t num_vkl_extensions;
	const char** vkl_extensions = vklGetRequiredInstanceExtensions(&num_vkl_extensions);

	// Merge both arrays of extensions:
	std::vector<const char*> all_required_extensions(glfw_extensions, glfw_extensions + num_glfw_extensions);
	all_required_extensions.insert(all_required_extensions.end(), vkl_extensions, vkl_extensions + num_vkl_extensions);

	// Perform a sanity check if all the extensions are really supported by Vulkan on
	// this system (if they are not, we have a problem):
	for (auto ext : all_required_extensions) {
		if (!hlpIsInstanceExtensionSupported(ext)) {
			VKL_EXIT_WITH_ERROR("Required extension \"" << ext << "\" is not supported");
		}
		VKL_LOG("Extension \"" << ext << "\" is supported");
	}

	return all_required_extensions;
}

uint32_t selectQueueFamilyIndex(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
	// Get the number of different queue families for the given physical device:
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

	// Get the queue families' data:
	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

	// TODO: Find a suitable queue family index and return it!

	for (uint32_t queue_family_index = 0u; queue_family_index < queue_family_count; ++queue_family_index) {
		// If this physical device supports a queue family which supports both, graphics and presentation
		//  => select this physical device
		if ((queue_families[queue_family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			// This queue supports graphics! Let's see if it also supports presentation:
			VkBool32 presentation_supported;
			vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, queue_family_index, surface, &presentation_supported);

			if (VK_TRUE == presentation_supported) {
				// We've found a suitable queue family
				return queue_family_index;
			}
		}
	}

	VKL_EXIT_WITH_ERROR("Unable to find a suitable queue family that supports graphics and presentation on the same queue.");
}
