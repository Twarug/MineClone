#pragma once

#include "Camera.h"
#include "RendererTypes.h"
#include "MineClone/Config.h"
#include "MineClone/Config.h"
#include "MineClone/Config.h"
#include "MineClone/Config.h"

namespace mc
{
    class RendererAPI
    {
    public:
        static void Init();
        static void Deinit();
        static void Wait();

        static void BeginFrame(float deltaTime, const Camera& camera);
        static void EndFrame();
        
        static void Resize(u32 width, u32 height);


        static Ref<Texture> LoadTexture(const std::string& filePath);
        static Ref<Texture> CreateTexture(u32 width, u32 height);

        static void DeleteTexture(Ref<Texture> texture);


        static Ref<AllocatedImage> CreateImage(u32 width, u32 height);
        
        static void DeleteImage(Ref<AllocatedImage> image);
        
        template<typename Vertex>
        static Ref<AllocatedBuffer> CreateVertexBuffer(std::span<Vertex> data);

        template<std::integral Index = u64>
        static Ref<AllocatedBuffer> CreateIndexBuffer(std::span<Index> data);

        static void DeleteBuffer(Ref<AllocatedBuffer> buffer);
        
        static void Draw(const Mat4& transform, Ref<AllocatedBuffer> vertexBuffer);
        static void Draw(const Mat4& transform, Ref<AllocatedBuffer> vertexBuffer, Ref<AllocatedBuffer> indexBuffer, u32 indicesCount);

        static void CopyBuffer(Ref<AllocatedBuffer> srcBuffer, Ref<AllocatedBuffer> dstBuffer, u64 size);
        static void CopyBuffer(Ref<AllocatedBuffer> srcBuffer, Ref<AllocatedImage> dstImage, u64 size);

        static void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

    private:
        static void CreateInstance();
        static void SetupDebugMessenger();
        static void CreateSurface();
        
        static void PickPhysicalDevice();
        static void CreateLogicalDevice();
        static void CreateSwapchain();
        static void CreateImageViews();
        static void CreateRenderPass();
        static void CreateDescriptorSetLayout();
        static void CreateDescriptorPool();
        static void CreateDescriptorSets();
        static void CreateGraphicsPipeline();
        static void CreateFramebuffers();
        static void CreateCommandPool();
        static void CreateCommandBuffers();
        static void CreateSyncObjects();

        static void CreateUniformBuffers();
        static void CreateImage(Ref<AllocatedImage> image, u32 width, u32 height);
        
        static void RecreateSwapchain();
        static void CleanupSwapchain();

        static Ref<AllocatedBuffer> CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                                 VkMemoryPropertyFlags properties, const void* data);
        static Ref<AllocatedBuffer> CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                                 VkMemoryPropertyFlags properties);
    };
}

#include "RendererAPI.tpp"