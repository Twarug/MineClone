#pragma once

#include "Event.h"

namespace mc
{
    class Window;
    
    // Window Events
    class WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent(const Window& window)
            : window(window) {}

        const Window& window;

        const char* GetName() const override { return "WindowCloseEvent"; }
    };

    class WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(const Window& window, u32 width, u32 height)
            : window(window), m_width(width), m_height(height) {}

        const Window& window;

        u32 GetWidth() const { return m_width; }
        u32 GetHeight() const { return m_height; }


        const char* GetName() const override { return "WindowResizeEvent"; }
        std::string ToString() const override {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << m_width << ", " << m_height;
            return  ss.str();
        }

    private:
        u32 m_width, m_height;
    };
}
