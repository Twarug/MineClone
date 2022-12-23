#pragma once

#include <vulkan/vulkan.h>

namespace mc
{
    class Buffer
    {
    public:
        static Ref<Buffer> CreateStageBuffer(u64 size);
        static Ref<Buffer> CreateStageBuffer(u64 size, const void* data);
        
        template <std::integral Index = u64>
        static Ref<Buffer> CreateIndexBuffer(std::span<Index> data);

        template <typename Vertex>
        static Ref<Buffer> CreateVertexBuffer(std::span<Vertex> data);

    private:
        Buffer() = default;

    public:
        ~Buffer() { Delete(); }
        
        void Delete();
        
    private:
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        void* mappedMemory = nullptr;

    private:
        static Ref<Buffer> CreateBuffer(u64 size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        static Ref<Buffer> CreateBuffer(u64 size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const void* data);
        
        friend class RendererAPI;
        friend class Material;
    };
}

#include "Buffer.tpp"