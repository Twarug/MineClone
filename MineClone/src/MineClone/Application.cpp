#include "mcpch.h"
#include "Application.h"

#include "Renderer/RendererAPI.h"
#include "Renderer/RendererTypes.h"


namespace mc
{
    static const std::vector<Vertex2D> VERTICES = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
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
        RendererAPI::Init();

        g_vertexBuffer = RendererAPI::CreateVertexBuffer(std::span(VERTICES));
        g_indexBuffer = RendererAPI::CreateIndexBuffer(std::span(INDICES));
        
        while (m_isRunning)
        {
            Update();
            
            {
                RendererAPI::BeginFrame();
                Render();
                RendererAPI::EndFrame();
            }
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
            float deltaTime = ((now - m_lastFrameTimePoint) / 1'000'000'000.f).count();

            // std::printf("FPS: %f\n", 1.f / deltaTime);
            
            m_lastFrameTimePoint = now;
        }
        
        m_window->Update();
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
    }

}
