#pragma once

#include "imgui.h"

namespace mc
{
    class GUI
    {
    public:
        static void Init();
        static void Deinit();
        
        static void BeginFrame();
        static void EndFrame();
    };
}
