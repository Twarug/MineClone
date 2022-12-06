#pragma once
#include "RendererUtils.h"

#include "GLFW/glfw3.h"

namespace mc::details
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

    SwapchainSupportDetails GetSwapchainSupportDetails(VkPhysicalDevice device)
    {
        SwapchainSupportDetails swapchain;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, g_state.surface, &swapchain.capabilities);

        u32 formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, g_state.surface, &formatCount, nullptr);
        
        swapchain.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, g_state.surface, &formatCount, swapchain.formats.data());

        u32 presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, g_state.surface, &presentModeCount, nullptr);

        swapchain.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, g_state.surface, &presentModeCount, swapchain.presentModes.data());
        return swapchain;
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
        swapchain = GetSwapchainSupportDetails(device);
            
        if (swapchain.formats.empty())
            return -1;

        if (swapchain.presentModes.empty())
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

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(g_state.physicalDevice, &memProperties);
        
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
}
