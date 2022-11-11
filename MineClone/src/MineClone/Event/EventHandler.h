#pragma once

#include <concepts>

namespace mc
{
    class Event;
    
    template<std::derived_from<Event>... Ts>
    class EventHandler : EventHandler<Ts>... {};
    
    template<std::derived_from<Event> T>
    class EventHandler<T>
    {
    protected:
        EventHandler();
        virtual ~EventHandler();

        EventHandler(const EventHandler& oth) = default;
        EventHandler& operator=(const EventHandler& oth) = default;
        
        EventHandler(EventHandler&& oth) = default;
        EventHandler& operator=(EventHandler&& oth) = default;
        
    protected:
        virtual void OnEvent(T& ev) = 0;

    public:
        static void Invoke(T& ev);
        
    private:
        static inline std::vector<EventHandler<T>*> s_handlers;
    };
}

#include "EventHandler.tpp"