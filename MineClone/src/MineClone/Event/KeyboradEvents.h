#pragma once

#include "Event.h"
#include "MineClone/Input/KeyCode.h"

namespace mc
{
    class KeyEvent : public Event
    {
    public:
        KeyCode GetKeyCode() const { return m_keyCode; }

    protected:
        KeyEvent(KeyCode keycode)
            : m_keyCode(keycode) {}

    private:
        KeyCode m_keyCode;
    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(KeyCode keycode, bool repeated)
            : KeyEvent(keycode), m_repeated(repeated) {}

        bool GetRepeatCount() const { return m_repeated; }

        const char* GetName() const override { return "KeyPressedEvent"; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << static_cast<u16>(GetKeyCode()) << " (" << m_repeated << " repeats)";
            return ss.str();
        }

    private:
        bool m_repeated;
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:
        KeyReleasedEvent(KeyCode keycode)
            : KeyEvent(keycode) {}

        const char* GetName() const override { return "KeyReleasedEvent"; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << static_cast<u16>(GetKeyCode());
            return ss.str();
        }
    };

    class KeyTypedEvent : public KeyEvent
    {
    public:
        KeyTypedEvent(KeyCode keycode)
            : KeyEvent(keycode) {}

        const char* GetName() const override { return "KeyTypedEvent"; }

        std::string ToString() const override {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << static_cast<u16>(GetKeyCode());
            return ss.str();
        }
    };
}
