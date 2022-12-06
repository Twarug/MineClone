#pragma once
#include "KeyCode.h"
#include "MouseCode.h"

namespace mc
{
    struct Key;
    struct Button;

    class Input
    {
    public:
        static Key GetKey(KeyCode keyCode);
        static Button GetButton(MouseCode mouseCode);

        static int2 GetMousePos();
        static float2 GetScrollDelta();
    };

    struct Key
    {
        bool down;
        bool pressed;
        bool up;
    };
    
    struct Button
    {
        bool down;
        bool pressed;
        bool up;
    };
}
