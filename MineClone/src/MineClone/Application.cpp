﻿#include "mcpch.h"
#include "Application.h"

#include "Renderer/RendererAPI.h"
#include "Renderer/RendererTypes.h"

#include "glm/gtc/matrix_transform.hpp"

namespace mc
{
    static const std::vector<Vertex3D> VERTICES = {
        {{-1, -1,  1}, {1.0f, 0.0f, 0.0f}}, //0
        {{ 1, -1,  1}, {0.0f, 1.0f, 0.0f}}, //1
        {{-1,  1,  1}, {0.0f, 0.0f, 1.0f}}, //2
        {{ 1,  1,  1}, {1.0f, 1.0f, 1.0f}}, //3

        {{-1, -1, -1}, {1.0f, 0.0f, 0.0f}}, //4
        {{ 1, -1, -1}, {0.0f, 1.0f, 0.0f}}, //5
        {{-1,  1, -1}, {0.0f, 0.0f, 1.0f}}, //6
        {{ 1,  1, -1}, {1.0f, 1.0f, 1.0f}}, //7
    };

    static const std::vector<u32> INDICES = {
        //Top
        2, 6, 7, 
        2, 7, 3,

        //Bottom
        0, 4, 5,
        0, 5, 1,

        //Left
        0, 2, 6,
        0, 6, 4,

        //Right
        1, 7, 3,
        1, 5, 7,

        //Front
        0, 3, 2,
        0, 1, 3,

        //Back
        4, 6, 7,
        4, 7, 5,
    };

    AllocatedBuffer g_vertexBuffer;
    AllocatedBuffer g_indexBuffer;
    
    Application::Application(std::string_view name)
        : name(name), m_isRunning(false)
    {
        s_instance = this;
    }

    
    void Application::Run()
    {
        m_isRunning = true;
        
        m_window = CreateScope<Window>(1280, 720, name);
        m_camera = CreateScope<Camera>(60.f, 1280, 720);
        m_camera->SetPos({0, -1, 10});
        m_camera->SetRot({30, 0});
        
        RendererAPI::Init();

        g_vertexBuffer = RendererAPI::CreateVertexBuffer(std::span(VERTICES));
        g_indexBuffer = RendererAPI::CreateIndexBuffer(std::span(INDICES));
        
        while (m_isRunning)
        {
            Update();

            RendererAPI::BeginFrame(m_deltaTime, *m_camera);
            Render();
            RendererAPI::EndFrame();
        }

        RendererAPI::Wait();
        
        RendererAPI::DeleteBuffer(g_vertexBuffer);
        RendererAPI::DeleteBuffer(g_indexBuffer);
        
        RendererAPI::Deinit();
    }

    void Application::Update()
    {
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            m_deltaTime = duration<float, seconds::period>(now - m_lastFrameTimePoint).count();
            m_lastFrameTimePoint = now;

            // printf("%f\n", .01f *m_deltaTime);
        }
        m_window->Update();
        m_camera->Update(m_deltaTime);
    }
    
    void Application::Render()
    {
        RendererAPI::Draw(g_vertexBuffer, g_indexBuffer, INDICES.size());
    }

    void Application::OnEvent(WindowCloseEvent& ev)
    {
        if(ev.window == *m_window)
            m_isRunning = false;
    }

    void Application::OnEvent(WindowResizeEvent& ev)
    {
        RendererAPI::Resize(ev.GetWidth(), ev.GetHeight());
        m_camera->OnResize(ev.GetWidth(), ev.GetHeight());
    }

}
