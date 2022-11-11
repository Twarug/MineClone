#pragma once
#include "EventHandler.h"

namespace mc
{
    template <std::derived_from<Event> T>
    EventHandler<T>::EventHandler()
    {
        s_handlers.push_back(this);
    }

    template <std::derived_from<Event> T>
    EventHandler<T>::~EventHandler()
    {
        s_handlers.erase(std::remove(s_handlers.begin(), s_handlers.end(), this));
    }

    template <std::derived_from<Event> T>
    void EventHandler<T>::Invoke(T& ev)
    {
        for(EventHandler<T>* handler : s_handlers)
            handler->OnEvent(ev);
    }
}
