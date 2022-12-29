#include "mcpch.h"
#include "Input.h"

#include "GLFW/glfw3.h"
#include "MineClone/Application.h"

namespace mc
{
    static Input g_input{};

    static std::array<Key, static_cast<u64>(KeyCode::_Count)> g_keys;
    static std::array<Button, static_cast<u64>(MouseCode::_Count)> g_mouseButtons;

    static float2 g_scrollOffset;

    Key Input::GetKey(KeyCode keyCode) {
        return g_keys[static_cast<u16>(keyCode)];
    }

    bool Input::GetAnyButtonDown() {
        for(Button& button : g_mouseButtons)
            if(button.down)
                return true;
        return false;
    }

    Button Input::GetButton(MouseCode mouseCode) {
        return g_mouseButtons[static_cast<u16>(mouseCode)];
    }

    int2 Input::GetMousePos() {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow());
        double2 pos;
        glfwGetCursorPos(window, &pos.x, &pos.y);
        return pos;
    }

    float2 Input::GetScrollDelta() {
        return g_scrollOffset;
    }

    void Input::OnEvent(AppUpdateEvent& ev) {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow());

        g_scrollOffset = {};

        for(u32 i = 0; i < g_keys.size(); i++) {
            Key& key = g_keys[i];
            bool pressed = glfwGetKey(window, i);

            if(key.down)
                key.down = false;

            if(key.up)
                key.up = false;

            if(pressed) {
                if(!key.pressed)
                    key.down = true;
            }
            else if(key.pressed)
                key.up = true;

            key.pressed = pressed;
        }

        for(u32 i = 0; i < g_mouseButtons.size(); i++) {
            Button& button = g_mouseButtons[i];
            bool pressed = glfwGetMouseButton(window, i);

            if(button.down)
                button.down = false;

            if(button.up)
                button.up = false;

            if(pressed) {
                if(!button.pressed)
                    button.down = true;
            }
            else if(button.pressed)
                button.up = true;

            button.pressed = pressed;
        }
    }

    void Input::OnEvent(MouseScrolledEvent& ev) {
        g_scrollOffset = ev.GetOffset();
    }
}
