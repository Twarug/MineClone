#include "mcpch.h"
#include "RendererAPI.h"

#include "MineClone/Application.h"

#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

namespace mc
{
    struct QueueFamilyIndices
    {
        u32 graphicsFamily = ~0u;
        u32 presentFamily = ~0u;

        bool IsComplete() const
        {
            return graphicsFamily != ~0u &&
                   presentFamily != ~0u;
        }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    
    struct GlobalState
    {
        VkInstance instance = VK_NULL_HANDLE;

        QueueFamilyIndices indices;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        VkDevice device;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSurfaceKHR surface;

        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchainImages;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;
        SwapchainSupportDetails swapchainSupportDetails;

        std::vector<VkImageView> swapchainImageViews;
        
        VkAllocationCallbacks* allocator = nullptr;

        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        
        std::vector<VkExtensionProperties> extensions;
    };
    static inline GlobalState g_state{};

    const std::array VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::array DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    #if NDEBUG
        bool g_enableValidationLayers = true;
    #else
        bool g_enableValidationLayers = false;
    #endif

    
    namespace details
    {
        VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
           VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
           VkDebugUtilsMessageTypeFlagsEXT messageType,
           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
           void* pUserData);
        
        bool CheckValidationLayerSupport();
        std::vector<const char*> GetRequiredExtensions();

        int GetDeviceScore(VkPhysicalDevice device, QueueFamilyIndices& indices, SwapchainSupportDetails& swapchain);

        std::vector<byte> ReadFile(const std::string& filename);
        
        VkShaderModule CreateShaderModule(const std::vector<byte>& code);
    }

    void CreateSwapchain();
    
    void RendererAPI::Init()
    {
        std::cout << "Renderer Init.\n";

        // Check for validation layers
        if (g_enableValidationLayers && !details::CheckValidationLayerSupport())
            MC_DEBUGBREAK();
        
        // Instance
        {
            VkApplicationInfo appInfo = {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = Application::Get().name.c_str(),
                .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                .pEngineName = "No Engine",
                .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                .apiVersion = VK_API_VERSION_1_3
            };
            
            auto extensions = details::GetRequiredExtensions();
                        
            VkInstanceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pApplicationInfo = &appInfo,
                .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
                .ppEnabledExtensionNames = extensions.data(),
            };

            if (g_enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
                createInfo.ppEnabledLayerNames = &*VALIDATION_LAYERS.begin();
            } else {
                createInfo.enabledLayerCount = 0;
            }
            
            if(vkCreateInstance(&createInfo, g_state.allocator, &g_state.instance) != VK_SUCCESS)
                throw std::runtime_error("Unable to create instance");
        }

        // Available Extensions
        {
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            g_state.extensions.resize(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, g_state.extensions.data());

            std::cout << "available extensions:\n";
            for (const auto& extension : g_state.extensions)
                std::cout << '\t' << extension.extensionName << '\n';
        }

        // Debug Setup
        if(g_enableValidationLayers) {
            VkDebugUtilsMessengerCreateInfoEXT createInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = details::DebugCallback,
                .pUserData = nullptr // Optional
            };

            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_state.instance, "vkCreateDebugUtilsMessengerEXT");
            if (func != nullptr)
                func(g_state.instance, &createInfo, g_state.allocator, &g_state.debugMessenger);
            else
                std::cerr << "Unable to create Vulkan Debug Callback\n";
        }

        // Create Surface
        {
            if (glfwCreateWindowSurface(g_state.instance, static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow()), g_state.allocator, &g_state.surface) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }
        }

        
        // Select Physical Device
        {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(g_state.instance, &deviceCount, nullptr);

            if (deviceCount == 0)
                throw std::runtime_error("failed to find GPUs with Vulkan support!");

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(g_state.instance, &deviceCount, devices.data());

            int bestScore = -1;
            for (const auto& device : devices)
            {
                QueueFamilyIndices indices;
                SwapchainSupportDetails swapchainSupportDetails;
                if (int score = details::GetDeviceScore(device, indices, swapchainSupportDetails); score >= 0 && bestScore < score) {
                    g_state.physicalDevice = device;
                    g_state.indices = indices;
                    g_state.swapchainSupportDetails = swapchainSupportDetails;
                    bestScore = score;
                }
            }

            if (g_state.physicalDevice == VK_NULL_HANDLE) {
                throw std::runtime_error("failed to find a suitable GPU!");
            }
        }

        // Create Device
        {
            float queuePriority = 1.f;
            
            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

            // Graphics Queue
            queueCreateInfos.push_back({
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = g_state.indices.graphicsFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            });

            // Present Queue
            queueCreateInfos.push_back({
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = g_state.indices.presentFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            });
            
            VkPhysicalDeviceFeatures deviceFeatures = {};

            VkDeviceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.data(),

                .enabledExtensionCount = static_cast<u32>(DEVICE_EXTENSIONS.size()),
                .ppEnabledExtensionNames = DEVICE_EXTENSIONS.data(),

                .pEnabledFeatures = &deviceFeatures,
            };

            // Backward compatibility
            {
                if (g_enableValidationLayers) {
                    createInfo.enabledLayerCount = static_cast<u32>(VALIDATION_LAYERS.size());
                    createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
                } else
                    createInfo.enabledLayerCount = 0;
            }

            if (vkCreateDevice(g_state.physicalDevice, &createInfo, g_state.allocator, &g_state.device) != VK_SUCCESS) {
                throw std::runtime_error("failed to create logical device!");
            }
            
            vkGetDeviceQueue(g_state.device, g_state.indices.graphicsFamily, 0, &g_state.graphicsQueue);
            vkGetDeviceQueue(g_state.device, g_state.indices.presentFamily, 0, &g_state.presentQueue);
        }

        CreateSwapchain();
    }

    void RendererAPI::Deinit()
    {
        std::cout << "Renderer Deinit.\n";

        
        for (auto imageView : g_state.swapchainImageViews)
            vkDestroyImageView(g_state.device, imageView, g_state.allocator);

        vkDestroySwapchainKHR(g_state.device, g_state.swapchain, g_state.allocator);
        vkDestroyDevice(g_state.device, g_state.allocator);

        if(g_enableValidationLayers)
        {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_state.instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr)
                func(g_state.instance, g_state.debugMessenger, g_state.allocator);
        }
        
        vkDestroySurfaceKHR(g_state.instance, g_state.surface, g_state.allocator);
        vkDestroyInstance(g_state.instance, g_state.allocator);
    }

    void CreateSwapchain()
    {        
        SwapchainSupportDetails& swapChainSupport = g_state.swapchainSupportDetails;
        
        VkSurfaceFormatKHR surfaceFormat = swapChainSupport.formats[0];
        for (const auto& availableFormat : swapChainSupport.formats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surfaceFormat = availableFormat;
                break;
            }

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto& availablePresentMode : swapChainSupport.presentModes)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = availablePresentMode;
                break;
            }

        VkExtent2D extent;
        if (swapChainSupport.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            extent = swapChainSupport.capabilities.currentExtent;
        else {
            int width, height;
            glfwGetFramebufferSize(static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow()), &width, &height);

            extent = {
                std::clamp(static_cast<u32>(width), swapChainSupport.capabilities.minImageExtent.width, swapChainSupport.capabilities.maxImageExtent.width),
                std::clamp(static_cast<u32>(height), swapChainSupport.capabilities.minImageExtent.height, swapChainSupport.capabilities.maxImageExtent.height)
            };
        }
        
        u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
            imageCount = swapChainSupport.capabilities.maxImageCount;
        
        {
            VkSwapchainCreateInfoKHR createInfo = {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .surface = g_state.surface,
                .minImageCount = imageCount,
                .imageFormat = surfaceFormat.format,
                .imageColorSpace = surfaceFormat.colorSpace,
                .imageExtent = extent,
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .preTransform = swapChainSupport.capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = presentMode,
                .clipped = VK_TRUE,
            };

            uint32_t queueFamilyIndices[] = {g_state.indices.graphicsFamily, g_state.indices.presentFamily};

            if (g_state.indices.graphicsFamily != g_state.indices.presentFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            } else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0; // Optional
                createInfo.pQueueFamilyIndices = nullptr; // Optional
            }
        
            if (vkCreateSwapchainKHR(g_state.device, &createInfo, g_state.allocator, &g_state.swapchain) != VK_SUCCESS)
                throw std::runtime_error("failed to create swap chain!");

            vkGetSwapchainImagesKHR(g_state.device, g_state.swapchain, &imageCount, nullptr);
            g_state.swapchainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(g_state.device, g_state.swapchain, &imageCount, g_state.swapchainImages.data());

            g_state.swapchainExtent = extent;
            g_state.swapchainImageFormat = surfaceFormat.format;
        }

        // Image Views
        {
            g_state.swapchainImageViews.resize(imageCount);

            for (u64 i = 0; i < imageCount; i++) {
                VkImageViewCreateInfo createInfo = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = g_state.swapchainImages[i],
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = g_state.swapchainImageFormat,
                    .components = {
                        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                    },
                    .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount =  1,
                    }
                };

                if (vkCreateImageView(g_state.device, &createInfo, g_state.allocator, &g_state.swapchainImageViews[i]) != VK_SUCCESS)
                    throw std::runtime_error("failed to create image views!");
            }
        }

        // Graphics Pipeline
        {
            auto vertShaderCode = details::ReadFile("assets/shaders/vert.spv");
            auto fragShaderCode = details::ReadFile("assets/shaders/frag.spv");
            
            VkShaderModule vertShaderModule = details::CreateShaderModule(vertShaderCode);
            VkShaderModule fragShaderModule = details::CreateShaderModule(fragShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vertShaderModule,
                .pName = "main",
            };

            VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = fragShaderModule,
                .pName = "main",
            };
            
            VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

            
            
            vkDestroyShaderModule(g_state.device, fragShaderModule, g_state.allocator);
            vkDestroyShaderModule(g_state.device, vertShaderModule, g_state.allocator);
        }
    }


    namespace details
    {
        VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {

            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

            return VK_FALSE;
        }
        
        bool CheckValidationLayerSupport() {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
            
            for (auto layerName : VALIDATION_LAYERS) {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers)
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }

                if (!layerFound)
                    return false;
            }

            return true;
        }
        
        std::vector<const char*> GetRequiredExtensions() {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            if (g_enableValidationLayers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        }

        int GetDeviceScore(VkPhysicalDevice device, QueueFamilyIndices& indices, SwapchainSupportDetails& swapchain)
        {
            int score = 0;
            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            // Device features requirements
            {
                if(!deviceFeatures.geometryShader)
                    return -1;
            }

            // Device Extension requirements
            {
                uint32_t extensionCount;
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

                std::vector<VkExtensionProperties> availableExtensions(extensionCount);
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

                std::set<std::string> requiredExtensions = {DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end()};
                
                for(const auto& [extensionName, specVersion] : availableExtensions)
                    requiredExtensions.erase(extensionName);

                if(!requiredExtensions.empty())
                    return -1;
            }
            
            // Device Queue Families requirements
            {
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

                std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

                u32 i = 0;
                for (const auto& queueFamily : queueFamilies) {
                    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                        indices.graphicsFamily = i;
                
                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, g_state.surface, &presentSupport);
                    if (presentSupport)
                        indices.presentFamily = i;

                    if (indices.IsComplete())
                        break;
                    
                    i++;
                }

                if(!indices.IsComplete())
                    return -1;
            }

            // Required Swapchain Support
            {
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, g_state.surface, &swapchain.capabilities);

                u32 formatCount;
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, g_state.surface, &formatCount, nullptr);
                
                if (formatCount == 0)
                    return -1;

                swapchain.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, g_state.surface, &formatCount, swapchain.formats.data());

                u32 presentModeCount;
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, g_state.surface, &presentModeCount, nullptr);

                if (presentModeCount == 0)
                    return -1;

                swapchain.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, g_state.surface, &presentModeCount, swapchain.presentModes.data());
            }

            switch (deviceProperties.deviceType)
            { 
                case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
                    score += 1;
                    break;
                
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    score += 10;
                    break;
                
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    score += 20;
                    break;
            }
            
            return score;
        }

        std::vector<byte> ReadFile(const std::string& filename) {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            if (!file.is_open())
                throw std::runtime_error("failed to open file!");

            size_t fileSize = file.tellg();
            std::vector<byte> buffer(fileSize);
            
            file.seekg(0);
            file.read(static_cast<char*>(static_cast<void*>(buffer.data())), fileSize);

            file.close();

            return buffer;
        }

        VkShaderModule CreateShaderModule(const std::vector<byte>& code) {
            VkShaderModuleCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t*>(code.data()),
            };

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(g_state.device, &createInfo, g_state.allocator, &shaderModule) != VK_SUCCESS)
                throw std::runtime_error("failed to create shader module!");

            return shaderModule;
        }
    }
}
