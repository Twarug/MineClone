#include "mcpch.h"
#include "RendererAPI.h"

#include "VulkanTypes.h"
#include "RendererTypes.h"
#include "MineClone/Application.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "VulkanUtils.h"

namespace mc
{
    static inline GlobalState g_state{};

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
        
        CreateUniformBuffers();
        // CreateDescriptorSetLayout();
        CreateDescriptorPool();
        // CreateDescriptorSets();

        // CreateGraphicsPipeline();
        CreateDepthBuffer();
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
        
        for(FrameData& frame : g_state.frames)
            DeleteBuffer(frame.uboBuffer);

        vkDestroyDescriptorPool(g_state.device, g_state.descriptorPool, g_state.allocator);

        vkDestroyRenderPass(g_state.device, g_state.renderPass, g_state.allocator);

        
        vkDestroyFence(g_state.device, g_state.uploadContext.uploadFence, g_state.allocator);
        vkDestroyCommandPool(g_state.device, g_state.uploadContext.commandPool, g_state.allocator);
        
        for(FrameData& frame : g_state.frames) {
            vkDestroySemaphore(g_state.device, frame.presentSemaphore, g_state.allocator);
            vkDestroySemaphore(g_state.device, frame.renderSemaphore, g_state.allocator);
            vkDestroyFence(g_state.device, frame.renderFence, g_state.allocator);

            vkDestroyCommandPool(g_state.device, frame.commandPool, g_state.allocator);
        }
        
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

    void RendererAPI::Wait()
    {
        vkDeviceWaitIdle(g_state.device);
    }


    void RendererAPI::BeginFrame(float deltaTime, const Camera& camera) {
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

        static float time = 0;
        time += deltaTime;
        UniformBufferObject ubo{{deltaTime, time, 0, 0}, camera.GetProjection(), camera.GetView()};
        memcpy(frame.uboBuffer->mappedMemory, &ubo, sizeof(ubo));

        vkResetCommandBuffer(frame.commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

        // Begin Command Buffer

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = 0, // Optional
            .pInheritanceInfo = nullptr, // Optional
        };

        if (vkBeginCommandBuffer(frame.commandBuffer, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("failed to begin recording command buffer!");

        std::array clearColor = {
            VkClearValue{{0.0f, 0.0f, 0.0f, 1.0f}},
            VkClearValue{.depthStencil = {1.f, 0}},
        };
        
        VkRenderPassBeginInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = g_state.renderPass,
            .framebuffer = g_state.swapchainFramebuffers[frame.currentImageIndex],
            .renderArea = {
                .offset = {0, 0},
                .extent = g_state.swapchainExtent,
            },
            .clearValueCount = (u32)clearColor.size(),
            .pClearValues = clearColor.data(),
        };
        vkCmdBeginRenderPass(frame.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {
            .x = 0.0f,
            .y = static_cast<float>(g_state.swapchainExtent.height),
            .width = static_cast<float>(g_state.swapchainExtent.width),
            .height = -static_cast<float>(g_state.swapchainExtent.height),
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

    Ref<Texture> RendererAPI::LoadTexture(const std::string& filePath)
    {
        i32 width, height, texChannels;

        stbi_set_flip_vertically_on_load(true);
        byte* pixels = stbi_load(filePath.c_str(), &width, &height, &texChannels, STBI_rgb_alpha);
        u64 imageSize = (u64)width * height * 4;

        if (!pixels)
            throw std::runtime_error("failed to load texture image!");

        Ref<Texture> texture = CreateTexture(width, height);
        Ref<AllocatedBuffer> stageBuffer = CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        vkMapMemory(g_state.device, stageBuffer->memory, 0, imageSize, 0, &data);
        memcpy(data, pixels, imageSize);
        vkUnmapMemory(g_state.device, stageBuffer->memory);
        
        CopyBuffer(stageBuffer, texture, imageSize);

        DeleteBuffer(stageBuffer);

        stbi_image_free(pixels);
        
        return texture;
    }

    Ref<Texture> RendererAPI::CreateTexture(u32 width, u32 height, VkFormat format)
    {
        Ref<Texture> texture = CreateRef<Texture>();

        CreateImage(texture, width, height, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

        VkImageViewCreateInfo viewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = texture->image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
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

        if (vkCreateImageView(g_state.device, &viewInfo, g_state.allocator, &texture->imageView) != VK_SUCCESS)
            throw std::runtime_error("failed to create image views!");

        VkSamplerCreateInfo samplerInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 0,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };
        
        if (vkCreateSampler(g_state.device, &samplerInfo, g_state.allocator, &texture->sampler) != VK_SUCCESS)
            throw std::runtime_error("failed to create texture sampler!");
        
        return texture;
    }

    void RendererAPI::DeleteTexture(Ref<Texture> texture)
    {
        vkDestroySampler(g_state.device, texture->sampler, g_state.allocator);
        vkDestroyImageView(g_state.device, texture->imageView, g_state.allocator);
        DeleteImage(texture);

        texture = {};
    }

    Ref<AllocatedImage> RendererAPI::CreateImage(u32 width, u32 height, VkFormat format)
    {
        Ref<AllocatedImage> image = CreateRef<AllocatedImage>();

        CreateImage(image, width, height, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
        
        return image;
    }

    void RendererAPI::DeleteImage(Ref<AllocatedImage> image)
    {
        vkFreeMemory(g_state.device, image->memory, g_state.allocator);
        vkDestroyImage(g_state.device, image->image, g_state.allocator);

        image = nullptr;
    }

    void RendererAPI::DeleteBuffer(Ref<AllocatedBuffer> buffer)
    {        
        vkDestroyBuffer(g_state.device, buffer->buffer, g_state.allocator);
        vkFreeMemory(g_state.device, buffer->memory, g_state.allocator);

        buffer = nullptr;
    }

    void RendererAPI::Draw(const Mat4& transform, Ref<Material> material, Ref<AllocatedBuffer> vertexBuffer)
    {
        FrameData& frame = g_state.GetCurrentFrame();

        MeshPushConstants pushConstants { transform };
        
        VkBuffer vertexBuffers[] = {vertexBuffer->buffer};
        VkDeviceSize offsets[] = {0};
        
        vkCmdPushConstants(frame.commandBuffer, material->m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &pushConstants);
        vkCmdBindVertexBuffers(frame.commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdDraw(frame.commandBuffer, 3, 1, 0, 0);
    }
    
    void RendererAPI::Draw(const Mat4& transform, Ref<Material> material, Ref<AllocatedBuffer> vertexBuffer, Ref<AllocatedBuffer> indexBuffer, u32 indicesCount)
    {
        FrameData& frame = g_state.GetCurrentFrame();
        
        MeshPushConstants pushConstants { transform };
        
        VkBuffer vertexBuffers[] = {vertexBuffer->buffer};
        VkDeviceSize offsets[] = {0};

        // vkCmdBindDescriptorSets(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_state.pipelineLayout, 0, 1, &frame.uboDescriptor, 0, nullptr);
        
        vkCmdPushConstants(frame.commandBuffer, material->m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &pushConstants);
        
        vkCmdBindVertexBuffers(frame.commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(frame.commandBuffer, indexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);
        
        vkCmdDrawIndexed(frame.commandBuffer, indicesCount, 1, 0, 0, 0);
    }

    void RendererAPI::CopyBuffer(Ref<AllocatedBuffer> srcBuffer, Ref<AllocatedBuffer> dstBuffer, u64 size)
    {
        ImmediateSubmit([=](VkCommandBuffer cmd) {
            VkBufferCopy copyRegion = {
                .srcOffset = 0, // Optional
                .dstOffset = 0, // Optional
                .size = size,
            };
            
            vkCmdCopyBuffer(cmd, srcBuffer->buffer, dstBuffer->buffer, 1, &copyRegion);
        });
    }

    void RendererAPI::CopyBuffer(Ref<AllocatedBuffer> srcBuffer, Ref<AllocatedImage> dstImage, u64 size)
    {
        ImmediateSubmit([=](VkCommandBuffer cmd) {
            VkImageSubresourceRange range = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            };

            VkImageMemoryBarrier imageBarrier_toTransfer = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,

                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,

                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                .image = dstImage->image,
                .subresourceRange = range,
            };

            //barrier the image into the transfer-receive layout
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

            VkBufferImageCopy copyRegion = {
                .bufferOffset = 0,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,
    
                .imageSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = 0,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
                .imageExtent = dstImage->extent,
            };

            //copy the buffer into the image
            vkCmdCopyBufferToImage(cmd, srcBuffer->buffer, dstImage->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
            
            VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

            imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            //barrier the image into the shader readable layout
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
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

    GlobalState& RendererAPI::GetState()
    {
        return g_state;
    }

    void RendererAPI::CreateInstance()
    {
        // Check for validation layers
        if (g_enableValidationLayers && !details::VulkanUtils::CheckValidationLayerSupport())
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
        
        auto extensions = details::VulkanUtils::GetRequiredExtensions();
                    
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
                .pfnUserCallback = details::VulkanUtils::DebugCallback,
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
            if (int score = details::VulkanUtils::GetDeviceScore(device, currentIndices, currentSwapchainSupportDetails); score >= 0 && bestScore < score) {
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
        
        VkAttachmentDescription depthAttachment = {
            .format = VK_FORMAT_D24_UNORM_S8_UINT,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
        std::array attachemnts = {colorAttachment, depthAttachment};

        VkAttachmentReference colorAttachmentRef = {
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentReference depthAttachmentRef = {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
        
        VkSubpassDescription subpass = {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pDepthStencilAttachment = &depthAttachmentRef,
        };

        VkSubpassDependency dependency = {
            
        };

        
        VkRenderPassCreateInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = (u32)attachemnts.size(),
            .pAttachments = attachemnts.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
        };

        if (vkCreateRenderPass(g_state.device, &renderPassInfo, g_state.allocator, &g_state.renderPass) != VK_SUCCESS)
            throw std::runtime_error("failed to create render pass!");
    }

    // void RendererAPI::CreateDescriptorSetLayout()
    // {        
    //     VkDescriptorSetLayoutBinding uboLayoutBinding = {
    //         .binding = 0,
    //         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    //         .descriptorCount = 1,
    //         .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    //         .pImmutableSamplers = nullptr,
    //     };
    //     
    //     VkDescriptorSetLayoutBinding samplerLayoutBinding = {
    //         .binding = 1,
    //         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    //         .descriptorCount = 1,
    //         .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    //         .pImmutableSamplers = nullptr,
    //     };
    //     
    //     std::array bindings = {uboLayoutBinding, samplerLayoutBinding};
    //     VkDescriptorSetLayoutCreateInfo layoutInfo {
    //         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
    //         .bindingCount = (u32)bindings.size(),
    //         .pBindings = bindings.data(),
    //     };
    //     
    //     if (vkCreateDescriptorSetLayout(g_state.device, &layoutInfo, g_state.allocator, &g_state.descriptorSetLayout) != VK_SUCCESS)
    //         throw std::runtime_error("failed to create descriptor set layout!");
    // }

    void RendererAPI::CreateDescriptorPool()
    {
        std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
		    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
        };

        VkDescriptorPoolCreateInfo poolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<u32>(FrameData::MAX_FRAMES_IN_FLIGHT),
            .poolSizeCount = (u32)poolSizes.size(),
            .pPoolSizes = poolSizes.data(),
        };

        if (vkCreateDescriptorPool(g_state.device, &poolInfo, g_state.allocator, &g_state.descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("failed to create descriptor pool!");
    }

    // void RendererAPI::CreateDescriptorSets()
    // {
    //     std::vector layouts(FrameData::MAX_FRAMES_IN_FLIGHT, g_state.descriptorSetLayout);
    //     VkDescriptorSetAllocateInfo allocInfo = {
    //         .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
    //         .descriptorPool = g_state.descriptorPool,
    //         .descriptorSetCount = 1,
    //         .pSetLayouts = layouts.data(),
    //     };
    //     
    //     for(FrameData& frame : g_state.frames) {
    //         if (vkAllocateDescriptorSets(g_state.device, &allocInfo, &frame.descriptor) != VK_SUCCESS)
    //             throw std::runtime_error("failed to allocate descriptor sets!");
    //         
    //         VkDescriptorBufferInfo bufferInfo = {
    //             .buffer = frame.uboBuffer->buffer,
    //             .offset = 0,
    //             .range = sizeof(UniformBufferObject),
    //         };
    //     
    //         VkWriteDescriptorSet descriptorWrite = {
    //             .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    //             .dstSet = frame.descriptor,
    //             .dstBinding = 0,
    //             .dstArrayElement = 0,
    //             .descriptorCount = 1,
    //             .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    //             .pImageInfo = nullptr,
    //             .pBufferInfo = &bufferInfo,
    //             .pTexelBufferView = nullptr,
    //         };
    //         
    //         vkUpdateDescriptorSets(g_state.device, 1, &descriptorWrite, 0, nullptr);
    //     }
    // }
    
    // void RendererAPI::CreateGraphicsPipeline()
    // {
    //     Graphics Pipeline
    //     auto vertShaderCode = details::ReadFile("assets/shaders/shader.vert.spv");
    //     auto fragShaderCode = details::ReadFile("assets/shaders/shader.frag.spv");
    //     
    //     VkShaderModule vertShaderModule = details::CreateShaderModule(vertShaderCode);
    //     VkShaderModule fragShaderModule = details::CreateShaderModule(fragShaderCode);
    //     
    //     VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
    //         .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    //         .stage = VK_SHADER_STAGE_VERTEX_BIT,
    //         .module = vertShaderModule,
    //         .pName = "main",
    //     };
    //     
    //     VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
    //         .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    //         .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    //         .module = fragShaderModule,
    //         .pName = "main",
    //     };
    //     
    //     VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    //     
    //     std::vector dynamicStates = {
    //         VK_DYNAMIC_STATE_VIEWPORT,
    //         VK_DYNAMIC_STATE_SCISSOR
    //     };
    //     
    //     VkPipelineDynamicStateCreateInfo dynamicState = {
    //         .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    //         .dynamicStateCount = static_cast<u32>(dynamicStates.size()),
    //         .pDynamicStates = dynamicStates.data(),
    //     };
    //     
    //     auto bindingDescription = Vertex3D::GetBindingDescription();
    //     auto attributeDescriptions = Vertex3D::GetAttributeDescriptions();
    //     
    //     VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
    //         .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    //         .vertexBindingDescriptionCount = 1,
    //         .pVertexBindingDescriptions = &bindingDescription,
    //         .vertexAttributeDescriptionCount = static_cast<u32>(attributeDescriptions.size()),
    //         .pVertexAttributeDescriptions = attributeDescriptions.data(),
    //     };
    //     
    //     VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
    //         .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    //         .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    //         .primitiveRestartEnable = VK_FALSE,
    //     };
    //     
    //     VkViewport viewport = {
    //         .x = 0.0f,
    //         .y = 0.0f,
    //         .width = static_cast<float>(g_state.swapchainExtent.width),
    //         .height = static_cast<float>(g_state.swapchainExtent.height),
    //         .minDepth = 0.0f,
    //         .maxDepth = 1.0f,
    //     };
    //     
    //     VkRect2D scissor = {
    //         .offset = {0, 0},
    //         .extent = g_state.swapchainExtent,
    //     };
    //     
    //     VkPipelineViewportStateCreateInfo viewportState = {
    //         .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    //         .viewportCount = 1,
    //         .pViewports = &viewport,
    //         .scissorCount = 1,
    //         .pScissors = &scissor,
    //     };
    //     
    //     VkPipelineRasterizationStateCreateInfo rasterizer = {
    //         .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    //         .depthClampEnable = VK_FALSE,
    //         .rasterizerDiscardEnable = VK_FALSE,
    //         .polygonMode = VK_POLYGON_MODE_FILL,
    //         
    //         .cullMode = VK_CULL_MODE_BACK_BIT,
    //         .frontFace = VK_FRONT_FACE_CLOCKWISE,
    //         
    //         .depthBiasEnable = VK_FALSE,
    //         .depthBiasConstantFactor = 0.0f, // Optional
    //         .depthBiasClamp = 0.0f, // Optional
    //         .depthBiasSlopeFactor = 0.0f, // Optional
    //         
    //         .lineWidth = 1.0f,
    //     };
    //     
    //     VkPipelineMultisampleStateCreateInfo multisampling = {
    //         .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    //         .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    //         .sampleShadingEnable = VK_FALSE,
    //         .minSampleShading = 1.0f, // Optional
    //         .pSampleMask = nullptr, // Optional
    //         .alphaToCoverageEnable = VK_FALSE, // Optional
    //         .alphaToOneEnable = VK_FALSE, // Optional
    //     };
    //     
    //     VkPipelineColorBlendAttachmentState colorBlendAttachment{
    //         .blendEnable = VK_TRUE,
    //         .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    //         .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    //         .colorBlendOp = VK_BLEND_OP_ADD,
    //         .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    //         .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    //         .alphaBlendOp = VK_BLEND_OP_ADD,
    //         .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    //     };
    //     
    //     VkPipelineColorBlendStateCreateInfo colorBlending = {
    //         .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    //         .logicOpEnable = VK_FALSE,
    //         .logicOp = VK_LOGIC_OP_COPY, // Optional
    //         .attachmentCount = 1,
    //         .pAttachments = &colorBlendAttachment,
    //         .blendConstants = {
    //             0.0f, 0.0f, 0.0f, 0.0f,
    //         }, // Optional
    //     };
    //     
    //     VkPushConstantRange pushConstant = {
    //         .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    //         .offset = 0,
    //         .size = sizeof(MeshPushConstants),
    //     };
    //     
    //     VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
    //         .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    //         .setLayoutCount = 1,
    //         .pSetLayouts = &g_state.descriptorSetLayout,
    //         .pushConstantRangeCount = 1,
    //         .pPushConstantRanges = &pushConstant,
    //     };
    //     
    //     if (vkCreatePipelineLayout(g_state.device, &pipelineLayoutInfo, g_state.allocator, &g_state.pipelineLayout) != VK_SUCCESS)
    //         throw std::runtime_error("failed to create pipeline layout!");
    //     
    //     VkGraphicsPipelineCreateInfo pipelineInfo = {
    //         .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    //         .stageCount = 2,
    //         .pStages = shaderStages,
    //         .pVertexInputState = &vertexInputInfo,
    //         .pInputAssemblyState = &inputAssembly,
    //         .pViewportState = &viewportState,
    //         .pRasterizationState = &rasterizer,
    //         .pMultisampleState = &multisampling,
    //         .pDepthStencilState = nullptr, // Optional
    //         .pColorBlendState = &colorBlending,
    //         .pDynamicState = &dynamicState,
    //         .layout = g_state.pipelineLayout,
    //         .renderPass = g_state.renderPass,
    //         .subpass = 0,
    //         .basePipelineHandle = VK_NULL_HANDLE, // Optional
    //         .basePipelineIndex = -1, // Optional
    //     };
    //     
    //     if (vkCreateGraphicsPipelines(g_state.device, VK_NULL_HANDLE, 1, &pipelineInfo, g_state.allocator, &g_state.graphicsPipeline) != VK_SUCCESS)
    //         throw std::runtime_error("failed to create graphics pipeline!");
    //     
    //     vkDestroyShaderModule(g_state.device, fragShaderModule, g_state.allocator);
    //     vkDestroyShaderModule(g_state.device, vertShaderModule, g_state.allocator);
    // }

    void RendererAPI::CreateFramebuffers()
    {
        u64 imageCount = g_state.swapchainImageViews.size();
        g_state.swapchainFramebuffers.resize(imageCount);

        for (size_t i = 0; i < imageCount; i++) {
            std::array attachments = {
                g_state.swapchainImageViews[i],
                g_state.depthView,
            };

            VkFramebufferCreateInfo framebufferInfo = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = g_state.renderPass,
                .attachmentCount = (u32)attachments.size(),
                .pAttachments = attachments.data(),
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

    void RendererAPI::CreateUniformBuffers()
    {
        u64 bufferSize = sizeof(UniformBufferObject);
        for (FrameData& frame : g_state.frames) {
            frame.uboBuffer = CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            vkMapMemory(g_state.device, frame.uboBuffer->memory, 0, bufferSize, 0, &frame.uboBuffer->mappedMemory);
        }
    }

    void RendererAPI::CreateImage(Ref<AllocatedImage> image, u32 width, u32 height, VkFormat format, VkImageUsageFlags usage)
    {
        image->format = format;
        image->extent = {
            .width = width,
            .height = height,
            .depth = 1,
        };
        
        VkImageCreateInfo imageInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = image->format,
            .extent = image->extent,
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        
        if (vkCreateImage(g_state.device, &imageInfo, g_state.allocator, &image->image) != VK_SUCCESS)
            throw std::runtime_error("failed to create image!");
        
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(g_state.device, image->image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = details::VulkanUtils::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(g_state.device, &allocInfo, g_state.allocator, &image->memory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate image memory!");

        vkBindImageMemory(g_state.device, image->image, image->memory, 0);
    }

    
    void RendererAPI::CreateDepthBuffer()
    {
        g_state.depthTexture = CreateRef<AllocatedImage>();
        CreateImage(g_state.depthTexture, g_state.swapchainExtent.width, g_state.swapchainExtent.height, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        
        VkImageViewCreateInfo viewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = g_state.depthTexture->image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_D24_UNORM_S8_UINT,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount =  1,
            }
        };

        if (vkCreateImageView(g_state.device, &viewInfo, g_state.allocator, &g_state.depthView) != VK_SUCCESS)
            throw std::runtime_error("failed to create image views!");
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

        g_state.swapchainSupportDetails = details::VulkanUtils::GetSwapchainSupportDetails(g_state.physicalDevice);
        
        CreateSwapchain();
        CreateImageViews();
        CreateDepthBuffer();
        CreateFramebuffers();
    }

    void RendererAPI::CleanupSwapchain()
    {
        vkDestroyImageView(g_state.device, g_state.depthView, g_state.allocator);
        DeleteImage(g_state.depthTexture);
        
        for (size_t i = 0; i < g_state.swapchainFramebuffers.size(); i++)
            vkDestroyFramebuffer(g_state.device, g_state.swapchainFramebuffers[i], g_state.allocator);

        for (size_t i = 0; i < g_state.swapchainImageViews.size(); i++)
            vkDestroyImageView(g_state.device, g_state.swapchainImageViews[i], g_state.allocator);

        vkDestroySwapchainKHR(g_state.device, g_state.swapchain, g_state.allocator);
    }

    Ref<AllocatedBuffer> RendererAPI::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const void* data)
    {
        Ref<AllocatedBuffer> buffer = CreateBuffer(size, usage, properties);
        
        void* mem = nullptr;
        vkMapMemory(g_state.device, buffer->memory, 0, size, 0, &mem);
        memcpy(mem, data, size);
        vkUnmapMemory(g_state.device, buffer->memory);
        
        return buffer;
    }

    Ref<AllocatedBuffer> RendererAPI::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        Ref<AllocatedBuffer> buffer = CreateRef<AllocatedBuffer>();

        VkBufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        
        vkCreateBuffer(g_state.device, &createInfo, g_state.allocator, &buffer->buffer);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(g_state.device, buffer->buffer, &memRequirements);
        
        VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = details::VulkanUtils::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
        };
        
        if (vkAllocateMemory(g_state.device, &allocInfo, g_state.allocator, &buffer->memory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate vertex buffer memory!");

        vkBindBufferMemory(g_state.device, buffer->buffer, buffer->memory, 0);

        return buffer;
    }
}
