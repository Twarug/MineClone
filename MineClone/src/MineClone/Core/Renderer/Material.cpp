#include "mcpch.h"
#include "Material.h"

#include "RendererAPI.h"
#include "VulkanTypes.h"
#include "VulkanUtils.h"

namespace mc
{
    Ref<Material> Material::Create(const std::string& name, const VertexDescription& vertexDescription) {
        auto& state = RendererAPI::GetState();

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        {
            VkDescriptorSetLayoutBinding uboLayoutBinding = {
                .binding = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = nullptr,
            };

            VkDescriptorSetLayoutBinding samplerLayoutBinding = {
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = nullptr,
            };

            std::array bindings = {uboLayoutBinding, samplerLayoutBinding};
            VkDescriptorSetLayoutCreateInfo layoutInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<u32>(bindings.size()),
                .pBindings = bindings.data(),
            };

            if(vkCreateDescriptorSetLayout(state.device, &layoutInfo, state.allocator, &descriptorSetLayout) != VK_SUCCESS)
                throw std::runtime_error("failed to create descriptor set layout!");

            std::vector layouts(FrameData::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
            VkDescriptorSetAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = state.descriptorPool,
                .descriptorSetCount = 1,
                .pSetLayouts = layouts.data(),
            };

            for(FrameData& frame : state.frames) {
                if(vkAllocateDescriptorSets(state.device, &allocInfo, &descriptorSet) != VK_SUCCESS)
                    throw std::runtime_error("failed to allocate descriptor sets!");

                VkDescriptorBufferInfo bufferInfo = {
                    .buffer = frame.uboBuffer->buffer,
                    .offset = 0,
                    .range = sizeof(UniformBufferObject),
                };

                VkWriteDescriptorSet descriptorWrite = {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = descriptorSet,
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pImageInfo = nullptr,
                    .pBufferInfo = &bufferInfo,
                    .pTexelBufferView = nullptr,
                };

                vkUpdateDescriptorSets(state.device, 1, &descriptorWrite, 0, nullptr);
            }

            // Graphics Pipeline
            auto vertShaderCode = details::VulkanUtils::ReadFile("assets/shaders/" + name + ".vert.spv");
            auto fragShaderCode = details::VulkanUtils::ReadFile("assets/shaders/" + name + ".frag.spv");

            VkShaderModule vertShaderModule = details::VulkanUtils::CreateShaderModule(vertShaderCode);
            VkShaderModule fragShaderModule = details::VulkanUtils::CreateShaderModule(fragShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = vertShaderModule,
                .pName = "main",
            };

            VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = fragShaderModule,
                .pName = "main",
            };

            VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

            std::vector dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };

            VkPipelineDynamicStateCreateInfo dynamicState = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                .dynamicStateCount = static_cast<u32>(dynamicStates.size()),
                .pDynamicStates = dynamicStates.data(),
            };

            auto bindingDescription = vertexDescription.bindingDescription;
            auto attributeDescriptions = vertexDescription.attributeDescriptions;

            VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 1,
                .pVertexBindingDescriptions = &bindingDescription,
                .vertexAttributeDescriptionCount = static_cast<u32>(attributeDescriptions.size()),
                .pVertexAttributeDescriptions = attributeDescriptions.data(),
            };

            VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .primitiveRestartEnable = VK_FALSE,
            };

            VkViewport viewport = {
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(state.swapchainExtent.width),
                .height = static_cast<float>(state.swapchainExtent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            };

            VkRect2D scissor = {
                .offset = {0, 0},
                .extent = state.swapchainExtent,
            };

            VkPipelineViewportStateCreateInfo viewportState = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .pViewports = &viewport,
                .scissorCount = 1,
                .pScissors = &scissor,
            };

            VkPipelineRasterizationStateCreateInfo rasterizer = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,

                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = VK_FRONT_FACE_CLOCKWISE,

                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0.0f, // Optional
                .depthBiasClamp = 0.0f, // Optional
                .depthBiasSlopeFactor = 0.0f, // Optional

                .lineWidth = 1.0f,
            };

            VkPipelineMultisampleStateCreateInfo multisampling = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 1.0f, // Optional
                .pSampleMask = nullptr, // Optional
                .alphaToCoverageEnable = VK_FALSE, // Optional
                .alphaToOneEnable = VK_FALSE, // Optional
            };

            VkPipelineDepthStencilStateCreateInfo depthStencil = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .depthTestEnable = VK_TRUE,
                .depthWriteEnable = VK_TRUE,
                .depthCompareOp = VK_COMPARE_OP_LESS,
                .depthBoundsTestEnable = VK_FALSE,
                .stencilTestEnable = VK_FALSE,
                .front = {}, // Optional
                .back = {}, // Optional
                .minDepthBounds = 0.0f, // Optional
                .maxDepthBounds = 1.0f, // Optional
            };

            VkPipelineColorBlendAttachmentState colorBlendAttachment{
                .blendEnable = VK_TRUE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };

            VkPipelineColorBlendStateCreateInfo colorBlending = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY, // Optional
                .attachmentCount = 1,
                .pAttachments = &colorBlendAttachment,
                .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}, // Optional
            };

            VkPushConstantRange pushConstant = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(MeshPushConstants),
            };

            VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = 1,
                .pSetLayouts = &descriptorSetLayout,
                .pushConstantRangeCount = 1,
                .pPushConstantRanges = &pushConstant,
            };

            if(vkCreatePipelineLayout(state.device, &pipelineLayoutInfo, state.allocator, &pipelineLayout) != VK_SUCCESS)
                throw std::runtime_error("failed to create pipeline layout!");

            VkGraphicsPipelineCreateInfo pipelineInfo = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .stageCount = 2,
                .pStages = shaderStages,
                .pVertexInputState = &vertexInputInfo,
                .pInputAssemblyState = &inputAssembly,
                .pViewportState = &viewportState,
                .pRasterizationState = &rasterizer,
                .pMultisampleState = &multisampling,
                .pDepthStencilState = &depthStencil, // Optional
                .pColorBlendState = &colorBlending,
                .pDynamicState = &dynamicState,
                .layout = pipelineLayout,
                .renderPass = state.renderPass,
                .subpass = 0,
                .basePipelineHandle = VK_NULL_HANDLE, // Optional
                .basePipelineIndex = -1, // Optional
            };

            if(vkCreateGraphicsPipelines(state.device, VK_NULL_HANDLE, 1, &pipelineInfo, state.allocator, &pipeline) != VK_SUCCESS)
                throw std::runtime_error("failed to create graphics pipeline!");

            vkDestroyShaderModule(state.device, fragShaderModule, state.allocator);
            vkDestroyShaderModule(state.device, vertShaderModule, state.allocator);
        }
      
        return CreateRef(new Material(descriptorSetLayout, descriptorSet, pipeline, pipelineLayout));
    }

    Material::Material(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet descriptorSet, VkPipeline pipeline, VkPipelineLayout pipelineLayout)
        : m_descriptorSetLayout(descriptorSetLayout), m_descriptorSet(descriptorSet), m_pipeline(pipeline), m_pipelineLayout(pipelineLayout) {}

    Material::~Material() {
        auto& state = RendererAPI::GetState();

        vkDestroyDescriptorSetLayout(state.device, m_descriptorSetLayout, state.allocator);

        vkDestroyPipeline(state.device, m_pipeline, state.allocator);
        vkDestroyPipelineLayout(state.device, m_pipelineLayout, state.allocator);
    }

    void Material::Bind() {
        auto& state = RendererAPI::GetState();
        auto& frame = state.GetCurrentFrame();

        state.currentMaterial = shared_from_this();

        vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
        vkCmdBindDescriptorSets(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
    }

    void Material::SetTexture(Ref<Texture> texture) {
        auto& state = RendererAPI::GetState();

        VkDescriptorImageInfo imageInfo = {
            .sampler = texture->sampler,
            .imageView = texture->imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_descriptorSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };

        vkUpdateDescriptorSets(state.device, 1, &descriptorWrite, 0, nullptr);
    }
}
