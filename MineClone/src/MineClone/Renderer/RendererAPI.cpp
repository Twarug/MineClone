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
    
    struct GlobalState
    {
        VkInstance instance = VK_NULL_HANDLE;

        QueueFamilyIndices indices;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        VkDevice device;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSurfaceKHR surface;

        
        VkAllocationCallbacks* allocator = nullptr;

        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        
        std::vector<VkExtensionProperties> extensions;
    };
    static inline GlobalState g_state{};

    const std::vector VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
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

        int GetDeviceScore(VkPhysicalDevice device, QueueFamilyIndices& indices);
    }
    
    
    void RendererAPI::Init(Window& window)
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
                createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
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
            if (glfwCreateWindowSurface(g_state.instance, static_cast<GLFWwindow*>(window.GetNativeWindow()), g_state.allocator, &g_state.surface) != VK_SUCCESS) {
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
                if (int score = details::GetDeviceScore(device, indices); score >= 0 && bestScore < score) {
                    g_state.physicalDevice = device;
                    g_state.indices = indices;
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

                .pEnabledFeatures = &deviceFeatures,
            };

            // Backward compatibility
            {
                createInfo.enabledExtensionCount = 0;

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
    }

    void RendererAPI::Deinit()
    {
        std::cout << "Renderer Deinit.\n";
        
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

        int GetDeviceScore(VkPhysicalDevice device, QueueFamilyIndices& indices)
        {
            int score = 0;
            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            
            if(!deviceFeatures.geometryShader)
                return -1;


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
    }
}
