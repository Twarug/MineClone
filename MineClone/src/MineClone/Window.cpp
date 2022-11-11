#include "mcpch.h"
#include "Window.h"

#include <GLFW/glfw3.h>

#include "Event/EventHandler.h"
#include "Event/WindowEvents.h"

namespace mc
{
    Window::Window(u32 width, u32 height, std::string_view title)
        : m_size(width, height)
    {
        if(s_windowCount == 0)
            if(!glfwInit()){
                std::cerr << "Unable to initialize GLFW!!!";
                exit(-1);
            }

        GLFWwindow* window;
        m_nativeWindow = window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);

        glfwMakeContextCurrent(window);
        s_windowCount++;

        glfwSetWindowUserPointer(window, this);
        
        glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            WindowResizeEvent ev{*win, static_cast<u32>(width), static_cast<u32>(height)};
            EventHandler<WindowResizeEvent>::Invoke(ev);
        });

        glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
            Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            WindowCloseEvent ev{*win};
            EventHandler<WindowCloseEvent>::Invoke(ev);
        });
    }

    Window::~Window()
    {
        glfwDestroyWindow(static_cast<GLFWwindow*>(m_nativeWindow));
        
        if(s_windowCount == 1)
            glfwTerminate();

        s_windowCount--;
    }

    void Window::Update() const
    {
        glfwPollEvents();
        glfwSwapBuffers(static_cast<GLFWwindow*>(m_nativeWindow));
    }

    void Window::Resize(u32 width, u32 height)
    {
        glfwSetWindowSize(static_cast<GLFWwindow*>(m_nativeWindow), width, height);
        m_size.x = width;
        m_size.y = height;
    }

    bool Window::operator==(const Window& oth) const
    {
        return m_nativeWindow == oth.m_nativeWindow;
    }
}
