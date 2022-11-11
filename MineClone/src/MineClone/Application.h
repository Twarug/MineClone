#pragma once

#include <MineClone/Window.h>

namespace mc
{
    class Application
    {
    public:
        std::string name;
    
    public:
        Application(std::string_view name);
        
        void Run();

        void Close() { m_isRunning = false; }
        
    public:
        static Application& Get() { return *s_instance; }

    private:
        bool m_isRunning;
        Scope<Window> m_window;

    private:
        inline static Application* s_instance = nullptr;
    };
}
