#pragma once

#include "vulkan/vulkan.h"
#include "VulkanTypes.h"

namespace mc::details
{
    class VulkanUtils
    {
    public:
        VKAPI_ATTR static VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            void* pUserData);

        static bool CheckValidationLayerSupport();

        static std::vector<const char*> GetRequiredExtensions();

        static SwapchainSupportDetails GetSwapchainSupportDetails(VkPhysicalDevice device);

        static int GetDeviceScore(VkPhysicalDevice device, QueueFamilyIndices& indices,
                                  SwapchainSupportDetails& swapchain);

        static std::vector<byte> ReadFile(const std::string& filename);

        static VkShaderModule CreateShaderModule(const std::vector<byte>& code);

        static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
}
