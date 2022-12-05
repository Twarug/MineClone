#include "mcpch.h"
#include "RendererAPI.h"

#include "RendererTypes.h"
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
    
    struct FrameData
    {
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
        
	    VkSemaphore presentSemaphore = VK_NULL_HANDLE;
        VkSemaphore renderSemaphore = VK_NULL_HANDLE;
        VkFence renderFence = VK_NULL_HANDLE;
        
        // DeletionQueue frameDeletionQueue{};

        u32 currentImageIndex = 0;
        
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

        AllocatedBuffer cameraBuffer{};
        VkDescriptorSet globalDescriptor = VK_NULL_HANDLE;

        AllocatedBuffer objectBuffer{};
        VkDescriptorSet objectDescriptor = VK_NULL_HANDLE;
    };
    
    struct GlobalState
    {
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;

        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSwapchainKHR swapchain;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;

        VkRenderPass renderPass;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;
        
        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        std::vector<VkFramebuffer> swapchainFramebuffers;

        uint32_t currentFrame = 0;
        std::array<FrameData, FrameData::MAX_FRAMES_IN_FLIGHT> frames;

        bool swapchainNeedsRecreation = false;
        uint2 currentWindowSize;

        QueueFamilyIndices indices;
        SwapchainSupportDetails swapchainSupportDetails;
       
        VkAllocationCallbacks* allocator = nullptr;
        std::vector<VkExtensionProperties> extensions;

        UploadContext uploadContext;

    public:
        FrameData& GetCurrentFrame()
        {
            return frames[currentFrame];
        }
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

        SwapchainSupportDetails GetSwapchainSupportDetails(VkPhysicalDevice device);

        int GetDeviceScore(VkPhysicalDevice device, QueueFamilyIndices& indices, SwapchainSupportDetails& swapchain);

        std::vector<byte> ReadFile(const std::string& filename);
        
        VkShaderModule CreateShaderModule(const std::vector<byte>& code);
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    }

    
    void RendererAPI::Init()
    {
        std::cout << "Renderer Init.\n";
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapchain();
        CreateImageViews();
        CreateRenderPass();
        CreateGraphicsPipeline();
        CreateFramebuffers();
        CreateCommandPool();
        CreateCommandBuffers();
        CreateSyncObjects();
    }

    void RendererAPI::Deinit()
    {        
        std::cout << "Renderer Deinit.\n";
        vkDeviceWaitIdle(g_state.device);

        CleanupSwapchain();        
        
        vkDestroyPipeline(g_state.device, g_state.graphicsPipeline, g_state.allocator);        
        vkDestroyPipelineLayout(g_state.device, g_state.pipelineLayout, g_state.allocator);

        vkDestroyRenderPass(g_state.device, g_state.renderPass, g_state.allocator);

        
        vkDestroyFence(g_state.device, g_state.uploadContext.uploadFence, g_state.allocator);
        vkDestroyCommandPool(g_state.device, g_state.uploadContext.commandPool, g_state.allocator);
        
        for(u64 i = 0; i < FrameData::MAX_FRAMES_IN_FLIGHT; i++) {
            FrameData& frame = g_state.frames[i];
            
            vkDestroySemaphore(g_state.device, frame.presentSemaphore, g_state.allocator);
            vkDestroySemaphore(g_state.device, frame.renderSemaphore, g_state.allocator);
            vkDestroyFence(g_state.device, frame.renderFence, g_state.allocator);

            vkDestroyCommandPool(g_state.device, frame.commandPool, g_state.allocator);
        }
        
        vkDestroyDevice(g_state.device, g_state.allocator);

        if(g_enableValidationLayers)
        {
            auto func = PFN_vkDestroyDebugUtilsMessengerEXT(vkGetInstanceProcAddr(g_state.instance, "vkDestroyDebugUtilsMessengerEXT"));
            if (func != nullptr)
                func(g_state.instance, g_state.debugMessenger, g_state.allocator);
        }
        
        vkDestroySurfaceKHR(g_state.instance, g_state.surface, g_state.allocator);
        vkDestroyInstance(g_state.instance, g_state.allocator);
    }

    void RendererAPI::Wait()
    {
        vkDeviceWaitIdle(g_state.device);
    }


    void RendererAPI::BeginFrame()
    {
        FrameData& frame = g_state.GetCurrentFrame();
        vkWaitForFences(g_state.device, 1, &frame.renderFence, true, std::numeric_limits<u64>::max());
        vkResetFences(g_state.device, 1, &frame.renderFence);

        VkResult result = vkAcquireNextImageKHR(g_state.device, g_state.swapchain, UINT64_MAX, frame.renderSemaphore, VK_NULL_HANDLE, &frame.currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapchain();
            return;
        }
        
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            throw std::runtime_error("failed to acquire swap chain image!");


        vkResetCommandBuffer(frame.commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

        // Begin Command Buffer

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = 0, // Optional
            .pInheritanceInfo = nullptr, // Optional
        };

        if (vkBeginCommandBuffer(frame.commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffer!");

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        
        VkRenderPassBeginInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = g_state.renderPass,
            .framebuffer = g_state.swapchainFramebuffers[frame.currentImageIndex],
            .renderArea = {
                .offset = {0, 0},
                .extent = g_state.swapchainExtent,
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor,
        };
        vkCmdBeginRenderPass(frame.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_state.graphicsPipeline);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(g_state.swapchainExtent.width),
            .height = static_cast<float>(g_state.swapchainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(frame.commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = g_state.swapchainExtent,
        };
        vkCmdSetScissor(frame.commandBuffer, 0, 1, &scissor);
    }

    void RendererAPI::EndFrame()
    {
        FrameData& frame = g_state.GetCurrentFrame();
        
        vkCmdEndRenderPass(frame.commandBuffer);

        if (vkEndCommandBuffer(frame.commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer!");

        
        VkSemaphore waitSemaphores[] = { frame.renderSemaphore };
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = { frame.presentSemaphore };

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,            
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,

            .commandBufferCount = 1,
            .pCommandBuffers = &frame.commandBuffer,

            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores,
        };


        if (vkQueueSubmit(g_state.graphicsQueue, 1, &submitInfo, frame.renderFence) != VK_SUCCESS)
            throw std::runtime_error("failed to submit draw command buffer!");

        VkSwapchainKHR swapChains[] = {g_state.swapchain};
        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,

            .swapchainCount = 1,
            .pSwapchains = swapChains,

            .pImageIndices = &frame.currentImageIndex,
        };

        VkResult result = vkQueuePresentKHR(g_state.presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || g_state.swapchainNeedsRecreation) {
            g_state.swapchainNeedsRecreation = false;
            RecreateSwapchain();
        } else if (result != VK_SUCCESS)
            throw std::runtime_error("failed to present swap chain image!");

        g_state.currentFrame = (g_state.currentFrame + 1) % FrameData::MAX_FRAMES_IN_FLIGHT;
    }


    void RendererAPI::Resize(u32 width, u32 height)
    {
        g_state.swapchainNeedsRecreation = true;
    }

    void RendererAPI::DeleteBuffer(AllocatedBuffer& buffer)
    {        
        vkDestroyBuffer(g_state.device, buffer.buffer, g_state.allocator);
        vkFreeMemory(g_state.device, buffer.memory, g_state.allocator);

        buffer = AllocatedBuffer();
    }

    void RendererAPI::Draw(const AllocatedBuffer& vertexBuffer)
    {
        FrameData& frame = g_state.GetCurrentFrame();
        
        VkBuffer vertexBuffers[] = {vertexBuffer.buffer};
        VkDeviceSize offsets[] = {0};
        
        vkCmdBindVertexBuffers(frame.commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdDraw(frame.commandBuffer, 3, 1, 0, 0);
    }

    void RendererAPI::CopyBuffer(const AllocatedBuffer& srcBuffer, const AllocatedBuffer& dstBuffer, u64 size)
    {
        ImmediateSubmit([=](VkCommandBuffer cmd) {
            VkBufferCopy copyRegion = {
                .srcOffset = 0, // Optional
                .dstOffset = 0, // Optional
                .size = size,
            };
            
            vkCmdCopyBuffer(cmd, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);
        });
    }

    void RendererAPI::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function)
    {
        VkCommandBuffer commandBuffer = g_state.uploadContext.commandBuffer;
        VkFence fence = g_state.uploadContext.uploadFence;
        //begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
        // VkCommandBufferBeginInfo cmdBeginInfo = vkinit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr,
        };

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffer!");


        function(commandBuffer);


        if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer!");

        VkSubmitInfo submit = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
        };

        //submit command buffer to the queue and execute it.
        // _renderFence will now block until the graphic commands finish execution
        if(vkQueueSubmit(g_state.graphicsQueue, 1, &submit, fence) != VK_SUCCESS)
            throw std::runtime_error("failed to submit draw command buffer!");

        vkWaitForFences(g_state.device, 1, &fence, true, std::numeric_limits<int>::max());
        vkResetFences(g_state.device, 1, &fence);

        vkResetCommandPool(g_state.device, g_state.uploadContext.commandPool, 0);
    }

    void RendererAPI::CreateInstance()
    {
        // Check for validation layers
        if (g_enableValidationLayers && !details::CheckValidationLayerSupport())
            MC_DEBUGBREAK();
        
        // Instance
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
    }

    void RendererAPI::SetupDebugMessenger()
    {
        // Debug Setup
        if(g_enableValidationLayers) {
            VkDebugUtilsMessengerCreateInfoEXT createInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = details::DebugCallback,
                .pUserData = nullptr // Optional
            };

            auto func = PFN_vkCreateDebugUtilsMessengerEXT(vkGetInstanceProcAddr(g_state.instance, "vkCreateDebugUtilsMessengerEXT"));
            if (func != nullptr)
                func(g_state.instance, &createInfo, g_state.allocator, &g_state.debugMessenger);
            else
                std::cerr << "Unable to create Vulkan Debug Callback\n";
        }
    }

    void RendererAPI::CreateSurface()
    {
        // Create Surface
        if (glfwCreateWindowSurface(g_state.instance, static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow()), g_state.allocator, &g_state.surface) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface!");
    }

    void RendererAPI::PickPhysicalDevice()
    {
        // Select Physical Device
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(g_state.instance, &deviceCount, nullptr);

        if (deviceCount == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(g_state.instance, &deviceCount, devices.data());

        int bestScore = -1;
        for (const auto& device : devices)
        {
            QueueFamilyIndices currentIndices;
            SwapchainSupportDetails currentSwapchainSupportDetails;
            if (int score = details::GetDeviceScore(device, currentIndices, currentSwapchainSupportDetails); score >= 0 && bestScore < score) {
                g_state.physicalDevice = device;
                g_state.indices = currentIndices;
                g_state.swapchainSupportDetails = currentSwapchainSupportDetails;
                bestScore = score;
            }
        }

        if (g_state.physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("failed to find a suitable GPU!");
    }

    void RendererAPI::CreateLogicalDevice()
    {
        // Create Device
        float queuePriority = 1.f;
        
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        // Graphics Queue
        queueCreateInfos.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = g_state.indices.graphicsFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        });

        if(g_state.indices.presentFamily != g_state.indices.graphicsFamily)
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
    
    void RendererAPI::CreateSwapchain()
    {        
        VkSurfaceFormatKHR surfaceFormat = g_state.swapchainSupportDetails.formats[0];
        for (const auto& availableFormat : g_state.swapchainSupportDetails.formats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surfaceFormat = availableFormat;
                break;
            }

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto& availablePresentMode : g_state.swapchainSupportDetails.presentModes)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = availablePresentMode;
                break;
            }

        VkExtent2D extent;
        if (g_state.swapchainSupportDetails.capabilities.currentExtent.width != std::numeric_limits<u32>::max())
            extent = g_state.swapchainSupportDetails.capabilities.currentExtent;
        else {
            int width, height;
            glfwGetFramebufferSize(static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow()), &width, &height);

            extent = {
                std::clamp(static_cast<u32>(width), g_state.swapchainSupportDetails.capabilities.minImageExtent.width, g_state.swapchainSupportDetails.capabilities.maxImageExtent.width),
                std::clamp(static_cast<u32>(height), g_state.swapchainSupportDetails.capabilities.minImageExtent.height, g_state.swapchainSupportDetails.capabilities.maxImageExtent.height)
            };
        }
        
        u32 imageCount = g_state.swapchainSupportDetails.capabilities.minImageCount + 1;
        if (g_state.swapchainSupportDetails.capabilities.maxImageCount > 0 && imageCount > g_state.swapchainSupportDetails.capabilities.maxImageCount)
            imageCount = g_state.swapchainSupportDetails.capabilities.maxImageCount;
        
        
        VkSwapchainCreateInfoKHR createInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = g_state.surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = g_state.swapchainSupportDetails.capabilities.currentTransform,
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

    void RendererAPI::CreateImageViews()
    {
        // Image Views
        u64 imageCount = g_state.swapchainImages.size();
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

    void RendererAPI::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment = {
            .format = g_state.swapchainImageFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };

        VkAttachmentReference colorAttachmentRef = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };
        
        VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
        };

        VkRenderPassCreateInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &colorAttachment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
        };

        if (vkCreateRenderPass(g_state.device, &renderPassInfo, g_state.allocator, &g_state.renderPass) != VK_SUCCESS)
            throw std::runtime_error("failed to create render pass!");
    }
    
    void RendererAPI::CreateGraphicsPipeline()
    {
        // Graphics Pipeline
        auto vertShaderCode = details::ReadFile("assets/shaders/shader.vert.spv");
        auto fragShaderCode = details::ReadFile("assets/shaders/shader.frag.spv");
        
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
        
        std::vector dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<u32>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data(),
        };

        auto bindingDescription = Vertex2D::GetBindingDescription();
        auto attributeDescriptions = Vertex2D::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<u32>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(g_state.swapchainExtent.width),
            .height = static_cast<float>(g_state.swapchainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = g_state.swapchainExtent,
        };

        VkPipelineViewportStateCreateInfo viewportState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor,
        };

        VkPipelineRasterizationStateCreateInfo rasterizer = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f, // Optional
            .depthBiasClamp = 0.0f, // Optional
            .depthBiasSlopeFactor = 0.0f, // Optional
            
            .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo multisampling = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f, // Optional
            .pSampleMask = nullptr, // Optional
            .alphaToCoverageEnable = VK_FALSE, // Optional
            .alphaToOneEnable = VK_FALSE, // Optional
        };

        VkPipelineColorBlendAttachmentState colorBlendAttachment{
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo colorBlending = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY, // Optional
            .attachmentCount = 1,
            .pAttachments = &colorBlendAttachment,
            .blendConstants = {
                0.0f, 0.0f, 0.0f, 0.0f,
            }, // Optional
        };
        
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0, // Optional
            .pSetLayouts = nullptr, // Optional
            .pushConstantRangeCount = 0, // Optional
            .pPushConstantRanges = nullptr, // Optional
        };

        if (vkCreatePipelineLayout(g_state.device, &pipelineLayoutInfo, g_state.allocator, &g_state.pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");

        VkGraphicsPipelineCreateInfo pipelineInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = nullptr, // Optional
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = g_state.pipelineLayout,
            .renderPass = g_state.renderPass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE, // Optional
            .basePipelineIndex = -1, // Optional
        };

        if (vkCreateGraphicsPipelines(g_state.device, VK_NULL_HANDLE, 1, &pipelineInfo, g_state.allocator, &g_state.graphicsPipeline) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");
        
        vkDestroyShaderModule(g_state.device, fragShaderModule, g_state.allocator);
        vkDestroyShaderModule(g_state.device, vertShaderModule, g_state.allocator);
    }

    void RendererAPI::CreateFramebuffers()
    {
        u64 imageCount = g_state.swapchainImageViews.size();
        g_state.swapchainFramebuffers.resize(imageCount);

        for (size_t i = 0; i < imageCount; i++) {
            VkImageView attachments[] = {
                g_state.swapchainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = g_state.renderPass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = g_state.swapchainExtent.width,
                .height = g_state.swapchainExtent.height,
                .layers = 1,
            };

            if (vkCreateFramebuffer(g_state.device, &framebufferInfo, g_state.allocator, &g_state.swapchainFramebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create framebuffer!");
        }
    }

    void RendererAPI::CreateCommandPool()
    {
        VkCommandPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = g_state.indices.graphicsFamily,
        };

        for(int i = 0; i < FrameData::MAX_FRAMES_IN_FLIGHT; i++){
            if (vkCreateCommandPool(g_state.device, &poolInfo, g_state.allocator, &g_state.frames[i].commandPool) != VK_SUCCESS)
                throw std::runtime_error("failed to create command pool!");
        }

        VkCommandPoolCreateInfo uploadPoolInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = g_state.indices.graphicsFamily,
        };
        
        if (vkCreateCommandPool(g_state.device, &uploadPoolInfo, g_state.allocator, &g_state.uploadContext.commandPool) != VK_SUCCESS)
            throw std::runtime_error("failed to create command pool!");
    }

    void RendererAPI::CreateCommandBuffers()
    {
        for(int i = 0; i < FrameData::MAX_FRAMES_IN_FLIGHT; i++){
            VkCommandBufferAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = g_state.frames[i].commandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            if (vkAllocateCommandBuffers(g_state.device, &allocInfo, &g_state.frames[i].commandBuffer) != VK_SUCCESS)
                throw std::runtime_error("failed to allocate command buffers!");
        }

        
        //allocate the default command buffer that we will use for rendering
        VkCommandBufferAllocateInfo cmdAllocInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = g_state.uploadContext.commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        if (vkAllocateCommandBuffers(g_state.device, &cmdAllocInfo, &g_state.uploadContext.commandBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffers!");
    }
    
    void RendererAPI::CreateSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VkFenceCreateInfo fenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        for(u64 i = 0; i < FrameData::MAX_FRAMES_IN_FLIGHT; i++)
            if (vkCreateSemaphore(g_state.device, &semaphoreInfo, g_state.allocator, &g_state.frames[i].renderSemaphore) != VK_SUCCESS ||
                vkCreateSemaphore(g_state.device, &semaphoreInfo, g_state.allocator, &g_state.frames[i].presentSemaphore) != VK_SUCCESS ||
                vkCreateFence(g_state.device, &fenceInfo, g_state.allocator, &g_state.frames[i].renderFence) != VK_SUCCESS)
                throw std::runtime_error("failed to create semaphores!");

        VkFenceCreateInfo uploadFenceInfo = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = 0,
        };
        if(vkCreateFence(g_state.device, &uploadFenceInfo, g_state.allocator, &g_state.uploadContext.uploadFence) != VK_SUCCESS)
            throw std::runtime_error("failed to create fence!");
    }
    
    void RendererAPI::RecreateSwapchain()
    {
        int width = 0, height = 0;
        GLFWwindow* window = (GLFWwindow*)Application::Get().GetMainWindow().GetNativeWindow();
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(g_state.device);

        CleanupSwapchain();

        g_state.swapchainSupportDetails = details::GetSwapchainSupportDetails(g_state.physicalDevice);
        
        CreateSwapchain();
        CreateImageViews();
        CreateFramebuffers();
    }

    void RendererAPI::CleanupSwapchain()
    {
        for (size_t i = 0; i < g_state.swapchainFramebuffers.size(); i++)
            vkDestroyFramebuffer(g_state.device, g_state.swapchainFramebuffers[i], g_state.allocator);

        for (size_t i = 0; i < g_state.swapchainImageViews.size(); i++)
            vkDestroyImageView(g_state.device, g_state.swapchainImageViews[i], g_state.allocator);

        vkDestroySwapchainKHR(g_state.device, g_state.swapchain, g_state.allocator);
    }

    AllocatedBuffer RendererAPI::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const void* data)
    {
        AllocatedBuffer buffer = CreateBuffer(size, usage, properties);
        
        void* mem = nullptr;
        vkMapMemory(g_state.device, buffer.memory, 0, size, 0, &mem);
        memcpy(mem, data, size);
        vkUnmapMemory(g_state.device, buffer.memory);
        
        return buffer;
    }

    AllocatedBuffer RendererAPI::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        AllocatedBuffer buffer{};

        VkBufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        
        vkCreateBuffer(g_state.device, &createInfo, g_state.allocator, &buffer.buffer);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(g_state.device, buffer.buffer, &memRequirements);
        
        VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = details::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
        };
        
        if (vkAllocateMemory(g_state.device, &allocInfo, g_state.allocator, &buffer.memory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate vertex buffer memory!");

        vkBindBufferMemory(g_state.device, buffer.buffer, buffer.memory, 0);

        return buffer;
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
}
