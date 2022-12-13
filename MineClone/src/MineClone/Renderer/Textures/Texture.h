#pragma once
#include <vulkan/vulkan.h>

namespace mc
{
    class AllocatedImage {
    public:
        VkImage image;
        VkDeviceMemory memory;
        VkExtent3D extent;
    };

    class Texture : public AllocatedImage
    {
    public:            
        VkImageView imageView;
        VkSampler sampler;
        VkDescriptorSet descriptor;
    };
}
