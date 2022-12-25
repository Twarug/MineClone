#include "mcpch.h"
#include "Application.h"

#include "Core/Input/Input.h"
#include "Game/World/Generator/ChunkGenerator.h"
#include "MineClone/Core/Event/ApplicationEvents.h"
#include "MineClone/Core/Renderer/RendererAPI.h"
#include "MineClone/Core/Renderer/RendererTypes.h"

namespace mc
{
    Mesh g_boundaryMesh;

    Mesh g_blockIndicator;
    HitInfo g_blockIndicatorInfo;
    
    
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

            RendererAPI::BeginFrame(m_deltaTime, *m_camera);
            Render();
            RendererAPI::EndFrame();
        }
        
        Cleanup();
    }

    void Application::Init() {
        // Application
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

        g_blockIndicator.SetIndices(std::span(Config::INDICES.data(), Config::INDICES.size()));
        g_blockIndicator.SetVertices(std::span(Config::VERTICES.front().data(), 6ull * 4ull));
    }

    void Application::Cleanup() {
        RendererAPI::Wait();
        g_boundaryMesh.Dispose();
        g_blockIndicator.Dispose();
        g_mat = nullptr;
        RendererAPI::DeleteTexture(g_texture);
        g_chunkMaterial = nullptr;
        RendererAPI::DeleteTexture(g_atlas);

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
        

        float2 rot = glm::radians(m_camera->GetRot());
        float xzLen = cos(rot.x);
        float3 dir = {xzLen * sin(-rot.y), sin(rot.x), -xzLen * cos(rot.y)};
        g_blockIndicatorInfo = m_world->RayCast(m_camera->GetPos(), dir, 10.f);
        if(g_blockIndicatorInfo.hit) {
            if(Input::GetButton(MouseCode::Button0).down)
                m_world->SetBlockState(g_blockIndicatorInfo.blockPos, BlockState());
            else if(Input::GetButton(MouseCode::Button1).down)
                m_world->SetBlockState(g_blockIndicatorInfo.blockPos  + int3(g_blockIndicatorInfo.hitNormal), BlockState(Block::STONE));
        }
        

        int3 currentChunkID = Chunk::ToChunkID(glm::floor(m_camera->GetPos()));
        if(currentChunkID != m_lastPlayerChunkID) {
            ChunkGenerator::UpdatePlayer(m_world, currentChunkID);
            m_lastPlayerChunkID = currentChunkID;
        }
    }

    void Application::Render() {
        g_chunkMaterial->Bind();
        m_world->Render();

        g_mat->Bind();
        if(g_blockIndicatorInfo.hit)
            g_blockIndicator.Render(glm::scale(glm::translate(Mat4{1}, float3(g_blockIndicatorInfo.blockPos)), float3(1.001f)));

        if(Input::GetKey(KeyCode::B).pressed) {
            if(Chunk* chunk = m_world->GetChunk(m_lastPlayerChunkID)) {
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
        m_camera->OnResize(ev.GetWidth(), ev.GetHeight());
    }
}
