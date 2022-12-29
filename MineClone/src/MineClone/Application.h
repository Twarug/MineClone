#pragma once

#include "Game/Player/Player.h"
#include "MineClone/Core/Window.h"

#include "MineClone/Core/Event/WindowEvents.h"

#include "Game/World/World.h"

namespace mc
{
    class Application final : EventHandler<WindowCloseEvent, WindowResizeEvent, WindowFocusEvent>
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
        void Init();
        void Cleanup();
        
        void Tick();
        void Update();
        void Render() const;
        
        
    protected:
        void OnEvent(WindowCloseEvent& ev) override;
        void OnEvent(WindowResizeEvent& ev) override;
        void OnEvent(WindowFocusEvent& ev) override;

    public:
        static Application& Get() { return *s_instance; }

    private:
        bool m_isRunning;
        bool m_isFocused;

        float m_deltaTime{};
        std::chrono::high_resolution_clock::time_point m_lastFrameTimePoint = std::chrono::high_resolution_clock::now();
        
        Scope<Window> m_window;

        
        Scope<World> m_world;
        Scope<Player> m_player;

    private:
        inline static Application* s_instance = nullptr;
    };
}
