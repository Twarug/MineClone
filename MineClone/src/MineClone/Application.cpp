#include "mcpch.h"
#include "Application.h"


namespace mc
{
    Application::Application(std::string_view name)
        : name(name), m_isRunning(false)
    {
        m_window = CreateScope<Window>(1280, 720, name);
    }

    
    void Application::Run()
    {
        m_isRunning = true;
        
        while (m_isRunning)
        {
            m_window->Update();
        }
    }

    void Application::OnEvent(WindowCloseEvent& ev)
    {
        if(ev.window == *m_window)
            m_isRunning = false;
    }

}