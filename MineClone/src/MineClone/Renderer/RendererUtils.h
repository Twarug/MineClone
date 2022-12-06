#pragma once

#include "RendererTypes.h"

namespace mc::details
{    
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
       VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
       VkDebugUtilsMessageTypeFlagsEXT messageType,
       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
       void* pUserData);
        
    bool CheckValidationLayerSupport();
    std::vector<const char*> GetRequiredExtensions();

    SwapchainSupportDetails GetSwapchainSupportDetails(VkPhysicalDevice device);

    int GetDeviceScore(VkPhysicalDevice device, QueueFamilyIndices& indices, SwapchainSupportDetails& swapchain);

    std::vector<byte> ReadFile(const std::string& filename);
        
    VkShaderModule CreateShaderModule(const std::vector<byte>& code);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
}