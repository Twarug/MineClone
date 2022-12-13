#pragma once

#include <vulkan/vulkan.h>

#include "Textures/Texture.h"

namespace mc
{
    class Material
    {
    public:
        static Ref<Material> Create(const std::string& name);

    private:
        Material(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet descriptorSet, VkPipeline pipeline,
                 VkPipelineLayout pipelineLayout);

    public:
        ~Material();

    public:
        void Bind();

        void SetTexture(Ref<Texture> texture);

    private:
        VkDescriptorSetLayout m_descriptorSetLayout;
        VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

        VkPipeline m_pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;


        friend class RendererAPI;
    };
}
