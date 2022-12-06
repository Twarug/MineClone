#include "mcpch.h"
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
        7, 2, 6,
        3, 2, 7,

        //Bottom
        4, 0, 5,
        5, 0, 1,

        //Left
        2, 0, 6,
        6, 0, 4,

        //Right
        7, 1, 3,
        5, 1, 7,

        //Front
        3, 0, 2,
        1, 0, 3,

        //Back
        6, 4, 7,
        7, 4, 5,
    };

    AllocatedBuffer g_vertexBuffer;
    AllocatedBuffer g_indexBuffer;
    
    Application::Application(std::string_view name)
        : name(name), m_isRunning(false)
    {
        s_instance = this;

        m_cubeTransform = glm::translate(Mat4{1}, {-2.5f, 0, 0});
        m_cube2Transform = glm::translate(Mat4{1}, {2.5f, 0, 0});
    }

    
    void Application::Run()
    {
        m_isRunning = true;
        
        m_window = CreateScope<Window>(1280, 720, name);
        m_camera = CreateScope<FirstPersonCamera>(60.f, 1280, 720);
        m_camera->SetPos({0, 1, 10});
        m_camera->SetRot({-30, 0});
        
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

        m_cubeTransform *= glm::rotate(Mat4{1}, glm::radians(10.f * m_deltaTime), {0, 1, 0});
    }
    
    void Application::Render()
    {        
        RendererAPI::Draw(m_cubeTransform, g_vertexBuffer, g_indexBuffer, (u32)INDICES.size());
        RendererAPI::Draw(m_cube2Transform, g_vertexBuffer, g_indexBuffer, (u32)INDICES.size());
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
