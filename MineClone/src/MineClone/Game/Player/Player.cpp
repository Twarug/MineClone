#include "mcpch.h"
#include "Player.h"

#include "MineClone/Core/Input/Input.h"
#include "MineClone/Game/World/World.h"
#include "MineClone/Game/World/Chunk.h"

#include "MineClone/Game/World/Generator/ChunkGenerator.h"

namespace mc
{    
    Player::Player(World& world)
        : m_world(world), m_camera(60.f, 1280, 720) {
        
        m_camera.SetPosition({0, 150, 30});
        m_camera.SetRotation({-30, 0});

        m_blockIndicator.SetIndices(std::span(Config::INDICES.data(), Config::INDICES.size()));
        m_blockIndicator.SetVertices(std::span(Config::VERTICES.front().data(), 6ull * 4ull));
    }

    Player::~Player() {
        m_blockIndicator.Dispose();
    }

    void Player::Render() const {
        if(m_blockIndicatorInfo.hit)
            m_blockIndicator.Render(glm::scale(glm::translate(Mat4{1}, float3(m_blockIndicatorInfo.blockPos)), float3(1.001f)));
    }

    void Player::Update(float deltaTime) {
        m_camera.Update(deltaTime);

        // Block Indicator
        float2 rot = glm::radians(GetRotation());
        float xzLen = cos(rot.x);
        float3 dir = {xzLen * sin(-rot.y), sin(rot.x), -xzLen * cos(rot.y)};
        m_blockIndicatorInfo = m_world.RayCast(GetPosition(), dir, 10.f);
        if(m_blockIndicatorInfo.hit) {
            if(Input::GetButton(MouseCode::Button0).down)
                m_world.SetBlockState(m_blockIndicatorInfo.blockPos, BlockState());
            else if(Input::GetButton(MouseCode::Button1).down)
                m_world.SetBlockState(m_blockIndicatorInfo.blockPos  + int3(m_blockIndicatorInfo.hitNormal), BlockState(Block::STONE));
        }

        // Chunk generation
        int3 currentChunkID = Chunk::ToChunkID(glm::floor(GetPosition()));
        if(currentChunkID != m_currentChunkID) {
            ChunkGenerator::UpdatePlayer(m_world, currentChunkID);
            m_currentChunkID = currentChunkID;
        }
    }
}
