#pragma once

#include "RendererTypes.h"

namespace mc
{
    class RendererAPI
    {
    public:
        static void Init();
        static void Deinit();
        static void Wait();

        static void BeginFrame(float deltaTime, const Mat4& projection, const Mat4& view);
        static void EndFrame();
        
        static void Resize(u32 width, u32 height);


        template<typename Vertex>
        static AllocatedBuffer CreateVertexBuffer(std::span<Vertex> data);

        template<std::integral Index = u64>
        static AllocatedBuffer CreateIndexBuffer(std::span<Index> data);

        static void DeleteBuffer(AllocatedBuffer& buffer);
        
        static void Draw(const AllocatedBuffer& vertexBuffer);
        static void Draw(const AllocatedBuffer& vertexBuffer, const AllocatedBuffer& indexBuffer, u32 indicesCount);

        static void CopyBuffer(const AllocatedBuffer& srcBuffer, const AllocatedBuffer& dstBuffer, u64 size);

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
        
        static void RecreateSwapchain();
        static void CleanupSwapchain();

        static AllocatedBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const void* data);
        static AllocatedBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    };
}

#include "RendererAPI.tpp"