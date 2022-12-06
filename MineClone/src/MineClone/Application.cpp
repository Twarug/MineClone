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

        //m_view = lookAt(float3(2.0f, 2.0f, 2.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 1.0f));
    }

    
    void Application::Run()
    {
        m_isRunning = true;
        
        m_window = CreateScope<Window>(1280, 720, name);
        m_projection = glm::perspective(glm::radians(45.0f), 1280 / 720.f, 0.1f, 1000.0f);
        RendererAPI::Init();

        g_vertexBuffer = RendererAPI::CreateVertexBuffer(std::span(VERTICES));
        g_indexBuffer = RendererAPI::CreateIndexBuffer(std::span(INDICES));
        
        while (m_isRunning)
        {
            Update();

            RendererAPI::BeginFrame(m_deltaTime, m_projection, m_view);
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
        m_window->Update();
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            m_deltaTime = duration<float, seconds::period>(now - m_lastFrameTimePoint).count();
            m_lastFrameTimePoint = now;

            // printf("%f\n", .01f *m_deltaTime);
        }
        
        static float3 pos{0, 0, -20};
        pos.z -= 1.f * m_deltaTime;
        m_view = translate(Mat4(1), pos);
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
        
        m_projection = glm::perspective(glm::radians(45.0f), (float)ev.GetWidth() / (float) ev.GetHeight(), 0.1f, 1000.0f);
    }

}
