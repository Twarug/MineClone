#pragma once
#include "MineClone/Core.h"

#include <vulkan/vulkan.h>
#include "vma/vk_mem_alloc.h"

namespace mc {

    struct AllocatedBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
    };

    struct AllocatedImage {
        VkImage image;
        VmaAllocation allocation;
    };

    struct MeshPushConstants {
        float4 data;
        Mat4 renderMatrix;
    };


    struct Material {
        VkDescriptorSet textureSet = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    };

    struct Texture {
        AllocatedImage image;
        VkImageView imageView;
    };

    struct RenderObject {
        // Mesh* mesh;

        Material* material;

        Mat4 transformMatrix;
    };
}