#pragma once
#include "MineClone/Core.h"

#include <vulkan/vulkan.h>

namespace mc {

    struct Vertex2D
    {
        float2 pos;
        float3 color;

        static VkVertexInputBindingDescription GetBindingDescription()
        {
            return {
                .binding = 0,
                .stride = sizeof(Vertex2D),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            };
        }

        static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
            return {{
                {
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = offsetof(Vertex2D, pos),
                },
                {
                    .location = 1,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex2D, color),
                },
            }};
        }
    };
    
    struct AllocatedBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
    };

    struct AllocatedImage {
        VkImage image;
        // VmaAllocation allocation;
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
    
    struct UploadContext {
        VkFence uploadFence;
        VkCommandPool commandPool;	
        VkCommandBuffer commandBuffer;
    };
}