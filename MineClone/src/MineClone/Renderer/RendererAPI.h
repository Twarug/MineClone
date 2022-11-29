#pragma once
#include "MineClone/Window.h"


namespace mc
{
    class RendererAPI
    {
    public:
        static void Init();
        static void Deinit();
        static void BeginFrame();
        static void EndFrame();
        
        static void Resize(u32 width, u32 height);

    private:
        static void CreateInstance();
        static void SetupDebugMessenger();
        static void CreateSurface();
        
        static void PickPhysicalDevice();
        static void CreateLogicalDevice();
        static void CreateSwapchain();
        static void CreateImageViews();
        static void CreateRenderPass();
        static void CreateGraphicsPipeline();
        static void CreateFramebuffers();
        static void CreateCommandPool();
        static void CreateCommandBuffers();
        static void CreateSyncObjects();
        
        static void RecreateSwapchain();
        static void CleanupSwapchain();
    };
}
