#include "mcpch.h"
#include "Application.h"

#include "Renderer/RendererAPI.h"
#include "Renderer/RendererTypes.h"

#include "glm/gtc/matrix_transform.hpp"

namespace mc
{
    static const std::vector<Vertex2D> VERTICES = {
        {{-5, -5}, {1.0f, 0.0f, 0.0f}},
        {{5, -5}, {0.0f, 1.0f, 0.0f}},
        {{5, 5}, {0.0f, 0.0f, 1.0f}},
        {{-5, 5}, {1.0f, 1.0f, 1.0f}},
    };

    static const std::vector<u32> INDICES = {
        0, 1, 2, 2, 3, 0
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
        m_camera->SetPos({0, 0, 20});
        
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
        RendererAPI::Draw(g_vertexBuffer, g_indexBuffer, 6);
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
