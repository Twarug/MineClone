#include "mcpch.h"
#include "Application.h"

#include "Core/Input/Input.h"
#include "Game/World/ChunkManager.h"
#include "Game/World/Generator/ChunkGenerator.h"
#include "MineClone/Core/Event/ApplicationEvents.h"
#include "MineClone/Core/Renderer/RendererAPI.h"
#include "MineClone/Core/Renderer/RendererTypes.h"

namespace mc
{
    Mesh g_boundaryMesh;    
    
    Ref<Texture> g_texture;
    Ref<Material> g_mat;

    Ref<Texture> g_atlas;
    Ref<Material> g_chunkMaterial;

    Application::Application(std::string_view name)
        : name(name), m_isRunning(false) {
        s_instance = this;
    }


    void Application::Run() {
        m_isRunning = true;

        Init();

        while(m_isRunning) {
            Update();

            RendererAPI::BeginFrame(m_deltaTime, m_player->GetCamera());
            Render();
            RendererAPI::EndFrame();
        }
        
        Cleanup();
    }

    void Application::Init() {
        // Application
        m_window = CreateScope<Window>(1280, 720, name);

        RendererAPI::Init();

        m_world = CreateScope<World>();
        m_player = CreateScope<Player>(*m_world);

        m_window->LockCursor();
        
        g_texture = RendererAPI::LoadTexture("assets/texture.png");
        auto des = Vertex3D::GetDescription();
        g_mat = Material::Create("default", des);
        g_mat->SetTexture(g_texture);
        
        g_atlas = RendererAPI::LoadTexture("assets/atlas.png", VK_FILTER_NEAREST);
        g_chunkMaterial = Material::Create("chunk", des);
        g_chunkMaterial->SetTexture(g_atlas);
        
        // Game

        ChunkGenerator::Init(2137);
        
        std::array<Vertex3D, 6ull * 4ull> verts{};

        i32 index = 0;
        for(const Facing& facing : Facing::FACINGS)
            for(i32 j = 0; j < 4; j++, index++) {
                verts[index] = Config::VERTICES[facing.index][(j + 2) % 4];
                verts[index].pos *= Config::CHUNK_SIZE;
                verts[index].color = facing.directionVec;
                verts[index].uv  *= facing.directionVec.y != 0 ? int2(Config::CHUNK_SIZE.xz) :
                                    facing.directionVec.x != 0 ? int2(Config::CHUNK_SIZE.yz) :
                                                                 int2(Config::CHUNK_SIZE.xy);
            }

        g_boundaryMesh.SetIndices(std::span(Config::INDICES.data(), Config::INDICES.size()));
        g_boundaryMesh.SetVertices(std::span(verts.data(), 6ull * 4ull));
    }

    void Application::Cleanup() {
        RendererAPI::Wait();
        g_boundaryMesh.Dispose();
        g_mat = nullptr;
        RendererAPI::DeleteTexture(g_texture);
        g_chunkMaterial = nullptr;
        RendererAPI::DeleteTexture(g_atlas);

        m_player.reset(nullptr);
        m_world.reset(nullptr);
        
        RendererAPI::Deinit();
    }

    void Application::Tick() {
        if(!m_isFocused)
            return;

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

        if(m_isFocused) {
            if(Input::GetKey(KeyCode::Escape).down) {
                m_window->UnlockCursor();
                m_isFocused = false;
            }
        }
        else {
            if(Input::GetAnyButtonDown()) {
                m_window->LockCursor();
                m_isFocused = true;
            }
                
        }

        if(m_isFocused)
            m_player->Update(m_deltaTime);

        ChunkManager::Update(*m_world);

        {
            AppUpdateEvent ev{m_deltaTime};
            EventHandler<AppUpdateEvent>::Invoke(ev);
        }
    }
    
    void Application::Render() const {
        g_chunkMaterial->Bind();
        m_world->Render();
        m_player->Render();

        g_mat->Bind();
        if(Input::GetKey(KeyCode::B).pressed) {
            if(Chunk* chunk = m_world->GetChunk(m_player->GetCurrentChunkID())) {
                int3 chunkID = chunk->GetID();
                g_boundaryMesh.Render(glm::translate(Mat4{1}, float3(chunkID * Config::CHUNK_SIZE)));
            }
        }
    }

    void Application::OnEvent(WindowCloseEvent& ev) {
        if(ev.window == *m_window)
            m_isRunning = false;
    }

    void Application::OnEvent(WindowResizeEvent& ev) {
        RendererAPI::Resize(ev.GetWidth(), ev.GetHeight());
    }

    void Application::OnEvent(WindowFocusEvent& ev) {
        m_isFocused = ev.GetFocused();
        if(ev.GetFocused())
            m_window->LockCursor();
        else
            m_window->UnlockCursor();
    }
}
