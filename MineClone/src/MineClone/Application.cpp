#include "mcpch.h"
#include "Application.h"

#include "Event/ApplicationEvents.h"
#include "Renderer/RendererAPI.h"
#include "Renderer/RendererTypes.h"

#include "glm/gtc/matrix_transform.hpp"

namespace mc
{
    static const std::vector VERTICES = {
        Vertex3D{{-1, -1, 1}, {}, {1.0f, 0.0f, 0.0f}, {0, 0}},
        //0
        Vertex3D{{1, -1, 1}, {}, {0.0f, 1.0f, 0.0f}, {1, 0}},
        //1
        Vertex3D{{-1, 1, 1}, {}, {0.0f, 0.0f, 1.0f}, {0, 1}},
        //2
        Vertex3D{{1, 1, 1}, {}, {1.0f, 1.0f, 1.0f}, {1, 1}},
        //3

        Vertex3D{{-1, -1, -1}, {}, {1.0f, 0.0f, 0.0f}, {0, 0}},
        //4
        Vertex3D{{1, -1, -1}, {}, {0.0f, 1.0f, 0.0f}, {1, 0}},
        //5
        Vertex3D{{-1, 1, -1}, {}, {0.0f, 0.0f, 1.0f}, {0, 1}},
        //6
        Vertex3D{{1, 1, -1}, {}, {1.0f, 1.0f, 1.0f}, {1, 1}},
        //7
    };

    static const std::vector<u32> INDICES = {
        //Top
        7,
        2,
        6,
        3,
        2,
        7,

        //Bottom
        4,
        0,
        5,
        5,
        0,
        1,

        //Left
        2,
        0,
        6,
        6,
        0,
        4,

        //Right
        7,
        1,
        3,
        5,
        1,
        7,

        //Front
        3,
        0,
        2,
        1,
        0,
        3,

        //Back
        6,
        4,
        7,
        7,
        4,
        5,
    };

    Ref<AllocatedBuffer> g_vertexBuffer;
    Ref<AllocatedBuffer> g_indexBuffer;

    Ref<Texture> g_texture;

    Ref<Material> g_mat;

    Application::Application(std::string_view name)
        : name(name), m_isRunning(false) {
        s_instance = this;

        m_cubeTransform = translate(Mat4{1}, {-2.5f, 0, 0});
        m_cube2Transform = translate(Mat4{1}, {2.5f, 0, 0});
    }


    void Application::Run() {
        m_isRunning = true;

        m_window = CreateScope<Window>(1280, 720, name);
        m_camera = CreateScope<FirstPersonCamera>(60.f, 1280, 720);
        m_camera->SetPos({0, 1, 10});
        m_camera->SetRot({-30, 0});

        RendererAPI::Init();

        g_vertexBuffer = RendererAPI::CreateVertexBuffer(std::span(VERTICES));
        g_indexBuffer = RendererAPI::CreateIndexBuffer(std::span(INDICES));

        g_texture = RendererAPI::LoadTexture("assets/texture.png");
        g_mat = Material::Create("default");
        g_mat->SetTexture(g_texture);

        while(m_isRunning) {
            Update();

            RendererAPI::BeginFrame(m_deltaTime, *m_camera);
            Render();
            RendererAPI::EndFrame();
        }

        RendererAPI::Wait();
        g_mat = nullptr;
        RendererAPI::DeleteTexture(g_texture);

        RendererAPI::DeleteBuffer(g_vertexBuffer);
        RendererAPI::DeleteBuffer(g_indexBuffer);

        RendererAPI::Deinit();
    }

    void Application::Update() {
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            m_deltaTime = duration<float, seconds::period>(now - m_lastFrameTimePoint).count();
            m_lastFrameTimePoint = now;

            // printf("%f\n", .01f *m_deltaTime);
        }

        m_window->Update();
        {
            AppUpdateEvent ev{m_deltaTime};
            EventHandler<AppUpdateEvent>::Invoke(ev);
        }

        m_camera->Update(m_deltaTime);
        m_cubeTransform *= rotate(Mat4{1}, glm::radians(10.f * m_deltaTime), {0, 1, 0});
    }

    void Application::Render() {
        g_mat->Bind();
        RendererAPI::Draw(m_cubeTransform, g_mat, g_vertexBuffer, g_indexBuffer, static_cast<u32>(INDICES.size()));
        RendererAPI::Draw(m_cube2Transform, g_mat, g_vertexBuffer, g_indexBuffer, static_cast<u32>(INDICES.size()));
    }

    void Application::OnEvent(WindowCloseEvent& ev) {
        if(ev.window == *m_window)
            m_isRunning = false;
    }

    void Application::OnEvent(WindowResizeEvent& ev) {
        RendererAPI::Resize(ev.GetWidth(), ev.GetHeight());
        m_camera->OnResize(ev.GetWidth(), ev.GetHeight());
    }
}
