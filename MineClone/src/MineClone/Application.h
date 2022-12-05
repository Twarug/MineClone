#pragma once

#include "MineClone/Window.h"

#include "MineClone/Event/WindowEvents.h"

namespace mc
{
    class Application final : EventHandler<WindowCloseEvent, WindowResizeEvent>
    {
    public:
        std::string name;
    
    public:
        explicit Application(std::string_view name);
        
        void Run();

        void Close() { m_isRunning = false; }


        Window& GetMainWindow() { return *m_window; }
        const Window& GetMainWindow() const { return *m_window; }
        
    private:
        void Update();
        void Render();
        
    protected:
        void OnEvent(WindowCloseEvent& ev) override;
        void OnEvent(WindowResizeEvent& ev) override;
        
    public:
        static Application& Get() { return *s_instance; }

    private:
        bool m_isRunning;

        std::chrono::high_resolution_clock::time_point m_lastFrameTimePoint;
        
        Scope<Window> m_window;

    private:
        inline static Application* s_instance = nullptr;
    };
}
