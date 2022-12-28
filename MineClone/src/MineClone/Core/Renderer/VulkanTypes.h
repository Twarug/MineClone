#pragma once

#include "RendererTypes.h"

#include <vulkan/vulkan.h>

namespace mc
{
    const std::array VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::array DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    };

#if NDEBUG
    inline bool g_enableValidationLayers = true;
#else
    inline bool g_enableValidationLayers = false;
#endif

    struct UploadContext
    {
        VkFence uploadFence;
        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
    };

    struct QueueFamilyIndices
    {
        u32 transferFamily = ~0u;
        u32 graphicsFamily = ~0u;
        u32 presentFamily = ~0u;

        bool IsComplete() const {
            return transferFamily != ~0u &&
                   graphicsFamily != ~0u &&
                   presentFamily != ~0u;
        }
    };

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct FrameData
    {
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        VkSemaphore presentSemaphore = VK_NULL_HANDLE;
        VkSemaphore renderSemaphore = VK_NULL_HANDLE;
        VkFence renderFence = VK_NULL_HANDLE;

        u32 currentImageIndex = 0;

        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

        Ref<Buffer> uboBuffer;
        
        std::vector<std::function<void()>> afterSubmit;
    };

    struct GlobalState
    {
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;

        VkQueue transferQueue;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSwapchainKHR swapchain;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;

        VkDescriptorPool descriptorPool;

        VkRenderPass renderPass;

        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        std::vector<VkFramebuffer> swapchainFramebuffers;

        Ref<Material> currentMaterial;

        uint32_t currentFrame = 0;
        std::array<FrameData, FrameData::MAX_FRAMES_IN_FLIGHT> frames;

        VkImageView depthView;
        Ref<AllocatedImage> depthTexture;

        bool swapchainNeedsRecreation = false;
        uint2 currentWindowSize;

        QueueFamilyIndices indices;
        SwapchainSupportDetails swapchainSupportDetails;

        VkAllocationCallbacks* allocator = nullptr;
        std::vector<VkExtensionProperties> extensions;

        UploadContext uploadContext;
        std::vector<std::function<void(VkCommandBuffer)>> nextFrameSubmits;

    public:
        FrameData& GetCurrentFrame() {
            return frames[currentFrame];
        }
    };
}
