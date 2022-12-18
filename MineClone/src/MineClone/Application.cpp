#include "mcpch.h"
#include "Application.h"

#include "MineClone/Core/Event/ApplicationEvents.h"
#include "MineClone/Core/Renderer/RendererAPI.h"
#include "MineClone/Core/Renderer/RendererTypes.h"

#include "glm/gtc/matrix_transform.hpp"

namespace mc
{
    Ref<AllocatedBuffer> g_vertexBuffer;
    Ref<AllocatedBuffer> g_indexBuffer;

    Ref<Texture> g_texture;

    Ref<Material> g_mat;

    Application::Application(std::string_view name)
        : name(name), m_isRunning(false) {
        s_instance = this;

        m_cubeTransform = translate(Mat4{1}, {15, 16, 15});
        m_cube2Transform = translate(Mat4{1}, {2.5f, 0, 0});
    }


    void Application::Run() {
        m_isRunning = true;

        m_window = CreateScope<Window>(1280, 720, name);
        m_camera = CreateScope<FirstPersonCamera>(60.f, 1280, 720);
        m_camera->SetPos({0, 15, 30});
        m_camera->SetRot({-30, 0});

        RendererAPI::Init();

        m_world = CreateScope<World>();
        
        g_vertexBuffer = RendererAPI::CreateVertexBuffer(std::span(Config::VERTICES.data(), Config::VERTICES.size() * Config::VERTICES[0].size()));
        g_indexBuffer = RendererAPI::CreateIndexBuffer(std::span(Config::INDICES.data(), Config::INDICES.size()));

        g_texture = RendererAPI::LoadTexture("assets/texture.png");
        auto des = Vertex3D::GetDescription();
        g_mat = Material::Create("default", des);
        g_mat->SetTexture(g_texture);

        m_world->Tick();
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

        m_world.reset(nullptr);
        
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
        // m_cubeTransform *= rotate(Mat4{1}, glm::radians(10.f * m_deltaTime), {0, 1, 0});
    }

    void Application::Render() {
        g_mat->Bind();
        m_world->Render();
        // RendererAPI::Draw(m_cubeTransform, g_vertexBuffer, g_indexBuffer, static_cast<u32>(Config::INDICES.size()));
        // RendererAPI::Draw(m_cube2Transform, g_vertexBuffer, g_indexBuffer, static_cast<u32>(Config::INDICES.size()));
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
