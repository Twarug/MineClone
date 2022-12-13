#pragma once
#include "MineClone/Core.h"

#include <vulkan/vulkan.h>

#include "Textures/Texture.h"
#include "Material.h"

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
    
    struct Vertex3D
    {
        float3 pos;
        float3 normal;
        float3 color;
        float2 uv;

        static VkVertexInputBindingDescription GetBindingDescription()
        {
            return {
                .binding = 0,
                .stride = sizeof(Vertex3D),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            };
        }

        static auto GetAttributeDescriptions() {
            return std::array {
                VkVertexInputAttributeDescription {
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex3D, pos),
                },
                VkVertexInputAttributeDescription {
                    .location = 1,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex3D, normal),
                },
                VkVertexInputAttributeDescription {
                    .location = 2,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = offsetof(Vertex3D, color),
                },
                VkVertexInputAttributeDescription {
                    .location = 3,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32_SFLOAT,
                    .offset = offsetof(Vertex3D, uv),
                },
            };
        }
    };
    
    struct AllocatedBuffer {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        void* mappedMemory = nullptr;
    };

    struct UniformBufferObject {
        float4 data;
        Mat4 proj;
        Mat4 view;
    };

    struct MeshPushConstants {
        Mat4 model;
    };
    
    struct RenderObject {
        // Mesh* mesh;

        Ref<Material> material;

        Mat4 transformMatrix;
    };
    
    struct UploadContext {
        VkFence uploadFence;
        VkCommandPool commandPool;	
        VkCommandBuffer commandBuffer;
    };

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
}