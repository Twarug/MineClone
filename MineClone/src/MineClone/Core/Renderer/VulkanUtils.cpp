#include "mcpch.h"
#include "VulkanUtils.h"

#include "RendererAPI.h"
#include "GLFW/glfw3.h"

#include "VulkanTypes.h"

namespace mc::details
{
    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanUtils::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        std::cerr << "validation layer (" << messageSeverity << "): " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    bool VulkanUtils::CheckValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for(auto layerName : VALIDATION_LAYERS) {
            bool layerFound = false;

            for(const auto& layerProperties : availableLayers)
                if(strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }

            if(!layerFound)
                return false;
        }

        return true;
    }

    std::vector<const char*> VulkanUtils::GetRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if(g_enableValidationLayers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    SwapchainSupportDetails VulkanUtils::GetSwapchainSupportDetails(VkPhysicalDevice device) {
        auto& state = RendererAPI::GetState();

        SwapchainSupportDetails swapchain;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, state.surface, &swapchain.capabilities);

        u32 formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, state.surface, &formatCount, nullptr);

        swapchain.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, state.surface, &formatCount, swapchain.formats.data());

        u32 presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, state.surface, &presentModeCount, nullptr);

        swapchain.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, state.surface, &presentModeCount,
                                                  swapchain.presentModes.data());
        return swapchain;
    }


    int VulkanUtils::GetDeviceScore(VkPhysicalDevice device, QueueFamilyIndices& indices,
                                    SwapchainSupportDetails& swapchain) {
        auto& state = RendererAPI::GetState();

        int score = 0;
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // Device features requirements
        {
            if(!deviceFeatures.geometryShader)
                return -1;

            if(deviceFeatures.samplerAnisotropy)
                score += 1;
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
            for(const auto& queueFamily : queueFamilies) {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, state.surface, &presentSupport);
                
                if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport) {
                    if(indices.graphicsFamily != indices.presentFamily || indices.graphicsFamily == ~0u) {
                        indices.graphicsFamily = i;
                        indices.presentFamily = i;
                    }
                }
                else if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    indices.graphicsFamily = i;
                else if(presentSupport)
                    indices.presentFamily = i;
                

                if(queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT && indices.transferFamily == ~0u)
                    indices.transferFamily = i;

                if(indices.IsComplete())
                    break;
                
                i++;
            }

            if(!indices.IsComplete())
                return -1;
        }

        // Required Swapchain Support
        swapchain = GetSwapchainSupportDetails(device);

        if(swapchain.formats.empty())
            return -1;

        if(swapchain.presentModes.empty())
            return -1;

        switch(deviceProperties.deviceType) {
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

    std::vector<byte> VulkanUtils::ReadFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if(!file.is_open())
            throw std::runtime_error("failed to open file!");

        size_t fileSize = file.tellg();
        std::vector<byte> buffer(fileSize);

        file.seekg(0);
        file.read(static_cast<char*>(static_cast<void*>(buffer.data())), (std::streamsize)fileSize);

        file.close();

        return buffer;
    }

    VkShaderModule VulkanUtils::CreateShaderModule(const std::vector<byte>& code) {
        auto& state = RendererAPI::GetState();

        VkShaderModuleCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode = reinterpret_cast<const uint32_t*>(code.data()),
        };

        VkShaderModule shaderModule;
        if(vkCreateShaderModule(state.device, &createInfo, state.allocator, &shaderModule) != VK_SUCCESS)
            throw std::runtime_error("failed to create shader module!");

        return shaderModule;
    }

    uint32_t VulkanUtils::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        auto& state = RendererAPI::GetState();

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(state.physicalDevice, &memProperties);

        for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;

        throw std::runtime_error("failed to find suitable memory type!");
    }

    VkFormat VulkanUtils::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        auto& state = RendererAPI::GetState();
        
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(state.physicalDevice, format, &props);
            
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                return format;
            
            if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                return format;
        }
        throw std::runtime_error("failed to find supported format!");
    }
}
