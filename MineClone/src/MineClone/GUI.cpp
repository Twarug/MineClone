#include "mcpch.h"
#include "GUI.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "Application.h"
#include "Core/Renderer/RendererAPI.h"
#include "Core/Renderer/VulkanTypes.h"

#include "vulkan/vulkan.h"

namespace mc
{
	VkDescriptorPool g_imguiPool;
	
    void GUI::Init() {
		GlobalState& state = RendererAPI::GetState();
    	
        //1: create descriptor pool for IMGUI
		// the size of the pool is very oversize, but it's copied from imgui demo itself.
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		VkDescriptorPool imguiPool;
		if(vkCreateDescriptorPool(state.device, &pool_info, nullptr, &imguiPool) != VK_SUCCESS)
			throw std::exception("Unable to create Descriptor pool for ImGui");


		// 2: initialize imgui library

		//this initializes the core structures of imgui
		ImGui::CreateContext();
    	ImGui::StyleColorsDark();

		//this initializes imgui for GLFW
		ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)Application::Get().GetMainWindow().GetNativeWindow(), true);

		//this initializes imgui for Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = state.instance;
		init_info.PhysicalDevice = state.physicalDevice;
		init_info.Device = state.device;
		init_info.Queue = state.graphicsQueue;
		init_info.DescriptorPool = imguiPool;
		init_info.MinImageCount = 3;
		init_info.ImageCount = 3;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&init_info, state.renderPass);

		//execute a gpu command to upload imgui font textures
		RendererAPI::SubmitImmediate([&](VkCommandBuffer cmd) {
			ImGui_ImplVulkan_CreateFontsTexture(cmd);
		});

		//clear font textures from cpu data
		ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    void GUI::Deinit() {
		GlobalState& state = RendererAPI::GetState();
    	
    	vkDestroyDescriptorPool(state.device, g_imguiPool, nullptr);
    	ImGui_ImplVulkan_Shutdown();
    }

    void GUI::BeginFrame() {
	    
    	//imgui new frame
    	ImGui_ImplVulkan_NewFrame();
    	ImGui_ImplGlfw_NewFrame();

    	ImGui::NewFrame();
    }

    void GUI::EndFrame() {
    	ImGui::Render();

    	FrameData& currentFrame = RendererAPI::GetState().GetCurrentFrame();
    	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), currentFrame.commandBuffer);
    }
}
