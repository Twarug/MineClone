#pragma once

#include <vulkan/vulkan.h>

// #include "RendererTypes.h"

namespace mc
{
    class Material
    {
    public:
        static Ref<Material> Create(const std::string& name);
        
    private:
        Material(VkDescriptorSet textureSet, VkPipeline pipeline, VkPipelineLayout pipelineLayout)
            : m_textureSet(textureSet), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout) {}
        
    public:
        // void SetTexture(const Texture& texture);
        
    private:
        VkDescriptorSet m_textureSet = VK_NULL_HANDLE;
        VkPipeline m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        friend class RendererAPI;
    };
}
