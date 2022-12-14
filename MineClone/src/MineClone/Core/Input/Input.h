#pragma once

#include "KeyCode.h"
#include "MouseCode.h"

#include "MineClone/Core/Event/EventHandler.h"
#include "MineClone/Core/Event/MouseEvents.h"
#include "MineClone/Core/Event/ApplicationEvents.h"

namespace mc
{
    struct Key;
    struct Button;

    class Input : EventHandler<AppUpdateEvent, MouseScrolledEvent>
    {
    public:
        static Key GetKey(KeyCode keyCode);

        static bool GetAnyButtonDown();
        static Button GetButton(MouseCode mouseCode);

        static int2 GetMousePos();
        static float2 GetScrollDelta();

    protected:
        void OnEvent(AppUpdateEvent& ev) override;
        void OnEvent(MouseScrolledEvent& ev) override;
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
