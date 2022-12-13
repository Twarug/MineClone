#pragma once

#include <string>

#include "EventHandler.h"

namespace mc
{
    class Event
    {
    public:
        Event() = default;
        virtual ~Event() = default;

        Event(const Event&) = delete;
        Event& operator=(const Event&) = delete;

        Event(Event&&) = delete;
        Event& operator=(Event&&) = delete;

        virtual const char* GetName() const = 0;
        virtual std::string ToString() const { return GetName(); }
    };
}
