#include "mcpch.h"
#include "Application.h"

#include "MineClone/Core/Event/ApplicationEvents.h"
#include "MineClone/Core/Renderer/RendererAPI.h"
#include "MineClone/Core/Renderer/RendererTypes.h"

namespace mc
{
    Ref<Texture> g_texture;

    Ref<Material> g_mat;

    Application::Application(std::string_view name)
        : name(name), m_isRunning(false) {
        s_instance = this;
    }


    void Application::Run() {
        m_isRunning = true;

        Init();

        while(m_isRunning) {
            Update();

            RendererAPI::BeginFrame(m_deltaTime, *m_camera);
            Render();
            RendererAPI::EndFrame();
        }
        
        Cleanup();
    }

    void Application::Init() {
        m_window = CreateScope<Window>(1280, 720, name);
        m_camera = CreateScope<FirstPersonCamera>(60.f, 1280, 720);
        m_camera->SetPos({0, 15, 30});
        m_camera->SetRot({-30, 0});

        RendererAPI::Init();

        m_world = CreateScope<World>();
        
        g_texture = RendererAPI::LoadTexture("assets/texture.png");
        auto des = Vertex3D::GetDescription();
        g_mat = Material::Create("default", des);
        g_mat->SetTexture(g_texture);
    }

    void Application::Cleanup() {
        RendererAPI::Wait();
        g_mat = nullptr;
        RendererAPI::DeleteTexture(g_texture);

        m_world.reset(nullptr);
        
        RendererAPI::Deinit();
    }

    void Application::Tick() {
        m_world->Tick();
    }

    void Application::Update() {
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            m_deltaTime = duration<float, seconds::period>(now - m_lastFrameTimePoint).count();
            m_lastFrameTimePoint = now;
            
            // printf("%d\n", (i32)(1.f / (m_deltaTime)));
        }

        m_window->Update();
        {
            AppUpdateEvent ev{m_deltaTime};
            EventHandler<AppUpdateEvent>::Invoke(ev);
        }

        m_camera->Update(m_deltaTime);

        static float timer = 0.f;
        static bool blockIndex = 0;
        if((timer += m_deltaTime) > 5.f) {
            timer -= 5.f;
            m_world->SetBlockState({16, 18, 16}, BlockState(blockIndex ? Block::DIRT : Block::STONE));
            blockIndex = !blockIndex;
        }
    }

    void Application::Render() {
        g_mat->Bind();
        m_world->Render();
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
