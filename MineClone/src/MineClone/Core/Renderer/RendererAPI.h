#pragma once

#include "Camera.h"
#include "RendererTypes.h"


namespace mc
{
    struct GlobalState;

    namespace details
    {
        class VulkanUtils;
    }

    class RendererAPI
    {
    public:
        static void Init();
        static void Deinit();
        static void Wait();

        static void BeginFrame(float deltaTime, const Camera& camera);
        static void EndFrame();

        static void Resize(u32 width, u32 height);


        static Ref<Texture> LoadTexture(const std::string& filePath, VkFilter filter = VK_FILTER_LINEAR);
        static Ref<Texture> CreateTexture(u32 width, u32 height, VkFilter filter = VK_FILTER_LINEAR, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

        static void DeleteTexture(Ref<Texture> texture);


        static Ref<AllocatedImage> CreateImage(u32 width, u32 height, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

        static void DeleteImage(Ref<AllocatedImage> image);

        static void Draw(const Mat4& transform, Ref<Buffer> vertexBuffer);
        static void Draw(const Mat4& transform, Ref<Buffer> vertexBuffer, Ref<Buffer> indexBuffer, u32 indicesCount);

        static void CopyBuffer(Ref<Buffer> srcBuffer, Ref<Buffer> dstBuffer, u64 size);
        static void CopyBuffer(Ref<Buffer> srcBuffer, Ref<AllocatedImage> dstImage, u64 size);

        static void SubmitImmediate(std::function<void(VkCommandBuffer cmd)>&& function);
        static void SubmitAfterFrame(std::function<void()>&& function);
        static void SubmitNextFrame(std::function<void(VkCommandBuffer cmd)>&& function);

    private:
        static GlobalState& GetState();

        static void CreateInstance();
        static void SetupDebugMessenger();
        static void CreateSurface();

        static void PickPhysicalDevice();
        static void CreateLogicalDevice();
        static void CreateSwapchain();
        static void CreateImageViews();
        static void CreateRenderPass();
        static void CreateDescriptorPool();
        static void CreateFramebuffers();
        static void CreateCommandPool();
        static void CreateCommandBuffers();
        static void CreateSyncObjects();

        static void CreateUniformBuffers();
        static void CreateImage(Ref<AllocatedImage> image, u32 width, u32 height, VkFormat format, VkImageUsageFlags usage);

        static void CreateDepthBuffer();
        static void RecreateSwapchain();
        static void CleanupSwapchain();

        friend details::VulkanUtils;

        friend class Material;
        friend class Texture;
        friend class Buffer;
        friend class Mesh;
    };
}