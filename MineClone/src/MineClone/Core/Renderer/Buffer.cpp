#include "mcpch.h"
#include "Buffer.h"

#include "VulkanTypes.h"
#include "RendererAPI.h"
#include "VulkanUtils.h"

namespace mc
{
    Ref<Buffer> Buffer::CreateStageBuffer(u64 size)
    {
        return CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
    
    Ref<Buffer> Buffer::CreateStageBuffer(u64 size, const void* data)
    {
        return CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data);
    }

    void Buffer::Delete()
    {
        auto& state = RendererAPI::GetState();
        if(buffer)
            vkDestroyBuffer(state.device, buffer, state.allocator);

        if(memory)
            vkFreeMemory(state.device, memory, state.allocator);

        buffer = nullptr;
        memory = nullptr;
        mappedMemory = nullptr;
    }

    

    Ref<Buffer> Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
        auto& state = RendererAPI::GetState();
        
        Ref<Buffer> buffer = Ref<Buffer>(new Buffer());

        VkBufferCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        vkCreateBuffer(state.device, &createInfo, state.allocator, &buffer->buffer);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(state.device, buffer->buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memRequirements.size,
            .memoryTypeIndex = details::VulkanUtils::FindMemoryType(memRequirements.memoryTypeBits, properties),
        };

        if(vkAllocateMemory(state.device, &allocInfo, state.allocator, &buffer->memory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate vertex buffer memory!");

        vkBindBufferMemory(state.device, buffer->buffer, buffer->memory, 0);

        return buffer;
    }
    
    Ref<Buffer> Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const void* data) {
        auto& state = RendererAPI::GetState();
        Ref<Buffer> buffer = CreateBuffer(size, usage, properties);

        vkMapMemory(state.device, buffer->memory, 0, size, 0, &buffer->mappedMemory);
        memcpy(buffer->mappedMemory, data, size);
        vkUnmapMemory(state.device, buffer->memory);
        buffer->mappedMemory = nullptr;

        return buffer;
    }
}
