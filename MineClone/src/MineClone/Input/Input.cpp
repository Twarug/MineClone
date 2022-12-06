#include "mcpch.h"
#include "Input.h"

#include "GLFW/glfw3.h"
#include "MineClone/Application.h"

namespace mc
{
    Key Input::GetKey(KeyCode keyCode)
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow());
        auto state = glfwGetKey(window, static_cast<int32_t>(keyCode));
        return {
            false,
            state == GLFW_PRESS,
            false,
        };
    }

    Button Input::GetButton(MouseCode mouseCode)
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, static_cast<int32_t>(mouseCode));
        return {
            false,
            state == GLFW_PRESS,
            false,
        };
    }

    int2 Input::GetMousePos()
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow());
        double2 pos;
        glfwGetCursorPos(window, &pos.x, &pos.y);
        return pos;
    }

    float2 Input::GetScrollDelta()
    {
        return {};
    }
}
