#include "mcpch.h"
#include "Player.h"

#include "MineClone/Core/Input/Input.h"
#include "MineClone/Core/Renderer/VulkanTypes.h"
#include "MineClone/Game/World/World.h"
#include "MineClone/Game/World/Chunk.h"
#include "MineClone/Game/World/ChunkManager.h"

namespace mc
{
    static std::vector<const Block*> g_blocksList;
    
    Player::Player(World& world)
        : m_world(world), m_camera(60.f, 1280, 720) {
        
        m_camera.SetPosition({0, 100, 30});
        m_camera.SetRotation({-30, 0});

        m_selectedBlockMesh.SetIndices(std::span(Config::INDICES.data(), Config::INDICES.size()));

        {
            std::vector<Vertex3D> vertices;
            vertices.append_range(std::span(Config::VERTICES.front().data(), 6ull * 4ull));

            // for(const Facing& facing : Facing::FACINGS)
                for(int i = 0; i < 4; i++) {
                    // vertices[facing.index * 4 + i].color = (float3)facing.directionVec / 4.f + float3{.75f};   
                    vertices[i].color = float3{0, .7f, 0};
                }
            
            m_blockIndicatorMesh.SetIndices(std::span(Config::INDICES.data(), Config::INDICES.size()));
            m_blockIndicatorMesh.SetVertices(std::span(vertices));

            m_blockIndicatorTexture = RendererAPI::LoadTexture("assets/block indicator.png", VK_FILTER_NEAREST);

            m_blockIndicatorMat = Material::Create("Block Indicator", Vertex3D::GetDescription());
            m_blockIndicatorMat->SetTexture(m_blockIndicatorTexture);
        }

        if(g_blocksList.empty()) {
            g_blocksList.append_range(Block::REGISTRY | std::views::values);
            g_blocksList.erase(std::ranges::remove(g_blocksList, &Block::AIR).begin());
        }
        SelectSlot(0);
    }

    Player::~Player() {
        m_selectedBlockMesh.Dispose();
        
        m_blockIndicatorMesh.Dispose();
        RendererAPI::DeleteTexture(m_blockIndicatorTexture);
        m_blockIndicatorMat = nullptr;
    }

    void Player::Render() const {

        Mat4 transform = inverse(RendererAPI::GetState().GetCurrentFrame().ubo.view);
        transform = glm::translate(transform, -float3{.5f, .5f, .5f});
        transform = glm::scale(transform, float3{.3f});
        transform = glm::translate(transform, float3{3, 0, -1.f});
        
        m_selectedBlockMesh.Render(transform);
        
        if(m_blockIndicatorInfo.hit) {
            m_blockIndicatorMat->Bind();
            transform = glm::translate(Mat4{1}, float3(m_blockIndicatorInfo.blockPos));
            transform = glm::translate(transform, float3(.5f));
            if(m_blockIndicatorInfo.hitNormal == float3{0, 1, 0}) // Top
                ;
            else if(m_blockIndicatorInfo.hitNormal == float3{0, -1, 0}) // Bottom
                transform = glm::rotate(transform, glm::radians(180.f), {1, 0, 0});
            else if(m_blockIndicatorInfo.hitNormal == float3{0, 0, 1}) // North
                transform = glm::rotate(transform, glm::radians(90.f), {1, 0, 0});
            else if(m_blockIndicatorInfo.hitNormal == float3{0, 0, -1}) // South
                transform = glm::rotate(transform, glm::radians(-90.f), {1, 0, 0});
            else if(m_blockIndicatorInfo.hitNormal == float3{1, 0, 0}) // East
                transform = glm::rotate(transform, glm::radians(-90.f), {0, 0, 1});
            else if(m_blockIndicatorInfo.hitNormal == float3{-1, 0, 0}) // West
                transform = glm::rotate(transform, glm::radians(90.f), {0, 0, 1});
            
            transform = glm::translate(transform, float3(-.5f));
            transform = glm::scale(transform, float3(1.002f));
            transform = glm::translate(transform, -float3{.001f});
            m_blockIndicatorMesh.Render(transform);
        }
    }

    void Player::Update(float deltaTime) {
        m_camera.Update(deltaTime);

        // Block Selection
        float scrollDelta = Input::GetScrollDelta().y;
        if(scrollDelta != 0.f) {
            if(scrollDelta < 0.f)
                SelectSlot((m_selectedBlockIndex - 1 + g_blocksList.size()) % (i32)g_blocksList.size());
            else
                SelectSlot((m_selectedBlockIndex + 1) % g_blocksList.size());
        }

        for(int i = 0; i < 10; i++)
            if(Input::GetKey((KeyCode)((i32)KeyCode::D1 + i)).down)
                SelectSlot(i % g_blocksList.size());
        
        // Block Indicator
        float2 rot = glm::radians(GetRotation());
        float xzLen = cos(rot.x);
        float3 dir = {xzLen * sin(-rot.y), sin(rot.x), -xzLen * cos(rot.y)};
        m_blockIndicatorInfo = m_world.RayCast(GetPosition(), dir, 10.f);
        if(m_blockIndicatorInfo.hit) {
            if(Input::GetButton(MouseCode::Button0).down)
                m_world.SetBlockState(m_blockIndicatorInfo.blockPos, BlockState());
            else if(Input::GetButton(MouseCode::Button1).down)
                m_world.SetBlockState(m_blockIndicatorInfo.blockPos  + int3(m_blockIndicatorInfo.hitNormal), BlockState(*g_blocksList[m_selectedBlockIndex]));
        }

        // Chunk generation
        int3 currentChunkID = Chunk::ToChunkID(glm::floor(GetPosition()));
        if(currentChunkID != m_currentChunkID) {
            ChunkManager::UpdatePlayer(m_world, currentChunkID);
            m_currentChunkID = currentChunkID;
        }
    }

    void Player::SelectSlot(int index) {
        m_selectedBlockIndex = index;
        const Block* block = g_blocksList[m_selectedBlockIndex];
            
        std::vector<Vertex3D> vertices;
        vertices.reserve(24);
            
        for(const Facing& facing : Facing::FACINGS) {
            auto face = Config::VERTICES[facing];
            float4 uvRect = block->GetTextureUVs(facing);
            std::array<float2, 4> uvs = {{
                uvRect.xy,
                uvRect.zy,
                uvRect.xw,
                uvRect.zw
            }};

            int vIndex = 0;
            for(Vertex3D v : face) {
                v.uv = uvs[vIndex++];
                vertices.push_back(v);
            }
        }
            
        m_selectedBlockMesh.SetVertices(std::span(vertices));
    }
}
