#pragma once
#include "MineClone/Core.h"

#include <vulkan/vulkan.h>

#include "Textures/Texture.h"
#include "Material.h"
#include "Buffer.h"

namespace mc
{
    struct VertexDescription
    {
        VkVertexInputBindingDescription bindingDescription;
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    };

    struct UniformBufferObject
    {
        float4 data;
        Mat4 proj;
        Mat4 view;
    };

    struct MeshPushConstants
    {
        Mat4 model;
    };

    struct RenderObject
    {
        // Ref<Mesh> mesh;

        Ref<Material> material;

        Mat4 transformMatrix;
    };
}
