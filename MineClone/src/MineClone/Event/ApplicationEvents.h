#pragma once
#include "Event.h"

namespace mc
{
    class AppTickEvent : public Event
    {
    public:
        AppTickEvent() = default;

        const char* GetName() const override { return "AppTickEvent"; }
    };

    class AppUpdateEvent : public Event
    {
    public:
        AppUpdateEvent(float deltaTime)
            : m_deltaTime(deltaTime) {}

        float GetDeltaTime() const { return  m_deltaTime; }
        
        const char* GetName() const override { return "AppUpdateEvent"; }
    private:
        float m_deltaTime;
    };

    class AppRenderEvent : public Event
    {
    public:
        AppRenderEvent() = default;
        
        const char* GetName() const override { return "AppRenderEvent"; }
    };
}
