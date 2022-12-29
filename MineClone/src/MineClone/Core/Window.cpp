#include "mcpch.h"
#include "Window.h"

#include <GLFW/glfw3.h>

#include "Event/EventHandler.h"
#include "Event/WindowEvents.h"
#include "Event/MouseEvents.h"
#include "Event/KeyboradEvents.h"

namespace mc
{
    Window::Window(u32 width, u32 height, std::string_view title)
        : m_size(width, height) {
        if(s_windowCount == 0) {
            if(!glfwInit()) {
                std::cerr << "Unable to initialize GLFW!!!";
                exit(-1);
            }

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        GLFWwindow* window;
        m_nativeWindow = window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
        s_windowCount++;

        glfwMakeContextCurrent(window);


        glfwSetWindowUserPointer(window, this);

        glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            WindowResizeEvent ev{*win, static_cast<u32>(width), static_cast<u32>(height)};
            EventHandler<WindowResizeEvent>::Invoke(ev);
        });

        
        glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focused) {
            Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            WindowFocusEvent ev{*win, (bool)focused};
            EventHandler<WindowFocusEvent>::Invoke(ev);
        });

        glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
            Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            WindowCloseEvent ev{*win};
            EventHandler<WindowCloseEvent>::Invoke(ev);
        });

        glfwSetKeyCallback(window, [](GLFWwindow*, int key, int scancode, int action, int mods) {
            switch(action) {
                case GLFW_PRESS:
                {
                    KeyPressedEvent ev(static_cast<KeyCode>(key), false);
                    EventHandler<KeyPressedEvent>::Invoke(ev);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent ev(static_cast<KeyCode>(key));
                    EventHandler<KeyReleasedEvent>::Invoke(ev);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent ev(static_cast<KeyCode>(key), true);
                    EventHandler<KeyPressedEvent>::Invoke(ev);
                    break;
                }
                default: ;
            }
        });

        glfwSetCharCallback(window, [](GLFWwindow*, unsigned int keycode) {
            KeyTypedEvent ev(static_cast<KeyCode>(keycode));
            EventHandler<KeyTypedEvent>::Invoke(ev);
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow*, int button, int action, int mods) {
            switch(action) {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent ev(static_cast<MouseCode>(button));
                    EventHandler<MouseButtonPressedEvent>::Invoke(ev);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent ev(static_cast<MouseCode>(button));
                    EventHandler<MouseButtonReleasedEvent>::Invoke(ev);
                    break;
                }
                default: ;
            }
        });

        glfwSetScrollCallback(window, [](GLFWwindow*, double xOffset, double yOffset) {
            MouseScrolledEvent ev({xOffset, yOffset});
            EventHandler<MouseScrolledEvent>::Invoke(ev);
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow*, double xPos, double yPos) {
            MouseMovedEvent ev({xPos, yPos});
            EventHandler<MouseMovedEvent>::Invoke(ev);
        });
    }

    Window::~Window() {
        glfwDestroyWindow(static_cast<GLFWwindow*>(m_nativeWindow));

        if(s_windowCount == 1)
            glfwTerminate();

        s_windowCount--;
    }

    void Window::Update() const {
        glfwPollEvents();
        glfwSwapBuffers(static_cast<GLFWwindow*>(m_nativeWindow));
    }

    void Window::Resize(u32 width, u32 height) {
        glfwSetWindowSize(static_cast<GLFWwindow*>(m_nativeWindow), width, height);
        m_size.x = width;
        m_size.y = height;
    }

    void Window::Focus() const {
        glfwFocusWindow((GLFWwindow*)m_nativeWindow);
    }
    
    void Window::LockCursor() const {
        glfwSetInputMode((GLFWwindow*)m_nativeWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void Window::UnlockCursor() const {
        glfwSetInputMode((GLFWwindow*)m_nativeWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    bool Window::operator==(const Window& oth) const {
        return m_nativeWindow == oth.m_nativeWindow;
    }
}
