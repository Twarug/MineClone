#pragma once
#include "MineClone/Window.h"


namespace mc
{
    class RendererAPI
    {
    public:
        static void Init(Window& window);
        static void Deinit();
    };
}
