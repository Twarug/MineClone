#pragma once
#include <vulkan/vulkan.h>

namespace mc
{
    class AllocatedImage
    {
    public:
        VkFormat format;
        VkExtent3D extent;

        VkImage image;
        VkDeviceMemory memory;
    };

    class Texture : public AllocatedImage
    {
    public:

    private:
        VkImageView imageView;
        VkSampler sampler;

        friend class Material;
        friend class RendererAPI;
    };
}
