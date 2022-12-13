#pragma once

#include "MineClone/Core.h"
#include "Event.h"
#include "MineClone/Input/MouseCode.h"

namespace mc
{
    class MouseMovedEvent : public Event
    {
    public:
        MouseMovedEvent(uint2 pos)
            : m_pos(pos) {}

        uint2 GetPos() const { return m_pos; }
        u32 GetX() const { return m_pos.x; }
        u32 GetY() const { return m_pos.y; }

        const char* GetName() const override { return "MouseMoveEvent"; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "MouseMoveEvent: " << m_pos.x << ", " << m_pos.y;
            return ss.str();
        }

    private:
        uint2 m_pos;
    };

    class MouseScrolledEvent : public Event
    {
    public:
        MouseScrolledEvent(float2 offset)
            : m_offset(offset) {}

        float2 GetOffset() const { return m_offset; }
        float GetXOffset() const { return m_offset.x; }
        float GetYOffset() const { return m_offset.y; }


        const char* GetName() const override { return "MouseScrolledEvent"; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "MouseScrolledEvent: " << m_offset.x << ", " << m_offset.y;
            return ss.str();
        }

    private:
        float2 m_offset;
    };

    class MouseButtonEvent : public Event
    {
    public:
        MouseCode GetMouseButton() const { return m_button; }

    protected:
        MouseButtonEvent(MouseCode button)
            : m_button(button) {}

    private:
        MouseCode m_button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(MouseCode button)
            : MouseButtonEvent(button) {}

        const char* GetName() const override { return "MouseButtonPressedEvent"; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "MouseButtonPressedEvent: " << static_cast<u16>(GetMouseButton());
            return ss.str();
        }
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(MouseCode button)
            : MouseButtonEvent(button) {}

        const char* GetName() const override { return "MouseButtonReleasedEvent"; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "MouseButtonReleasedEvent: " << static_cast<u16>(GetMouseButton());
            return ss.str();
        }
    };
}
