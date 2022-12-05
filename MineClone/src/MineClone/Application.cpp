﻿#include "mcpch.h"
#include "Application.h"

#include "Renderer/RendererAPI.h"


namespace mc
{
    Application::Application(std::string_view name)
        : name(name), m_isRunning(false)
    {
        s_instance = this;
    }

    
    void Application::Run()
    {
        m_isRunning = true;

        m_window = CreateScope<Window>(1280, 720, name);
        RendererAPI::Init();
        
        while (m_isRunning)
        {
            Update();
            
            {
                RendererAPI::BeginFrame();
                Render();
                RendererAPI::EndFrame();
            }
        }
        
        RendererAPI::Deinit();
    }

    void Application::Update()
    {
        m_window->Update();
    }

    void Application::Render()
    {
        
    }

    void Application::OnEvent(WindowCloseEvent& ev)
    {
        if(ev.window == *m_window)
            m_isRunning = false;
    }

    void Application::OnEvent(WindowResizeEvent& ev)
    {
        RendererAPI::Resize(ev.GetWidth(), ev.GetHeight());
    }

}
