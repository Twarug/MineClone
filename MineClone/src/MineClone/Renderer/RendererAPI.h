#pragma once
#include "MineClone/Window.h"


namespace mc
{
    class RendererAPI
    {
    public:
        static void Init();
        static void Deinit();
        static void RenderFrame();
        
        static void Resize(u32 width, u32 height);
    };
}
