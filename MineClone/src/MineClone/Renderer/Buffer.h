#pragma once

#include <vulkan/vulkan.h>

namespace mc
{
    struct AllocatedBuffer
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        void* mappedMemory = nullptr;
    };
}
