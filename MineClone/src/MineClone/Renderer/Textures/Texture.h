#pragma once
#include <vulkan/vulkan.h>

namespace mc
{
    class AllocatedImage {
    public:
        VkExtent3D extent;
        VkFormat format;
        
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
