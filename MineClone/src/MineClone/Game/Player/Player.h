#pragma once
#include "MineClone/Game/World/World.h"
#include "MineClone/FirstPersonCamera.h"

namespace mc
{   
    class Player
    {
    public:
        explicit Player(World& world);
        ~Player();

        Player(const Player&) = delete;
        Player& operator=(const Player&) = delete;
        Player(Player&&) = delete;
        Player& operator=(Player&&) = delete;

    public:

        void Render() const;
        void Update(float deltaTime);

    public:
        Camera& GetCamera() { return m_camera; }
        const Camera& GetCamera() const { return m_camera; }

        float3 GetPosition() const { return m_camera.GetPosition(); }
        float2 GetRotation() const { return m_camera.GetRotation(); }
        
        int3 GetCurrentChunkID() const { return m_currentChunkID; }

    private:
        World& m_world;
        FirstPersonCamera m_camera;

        int3 m_currentChunkID {};

        
        Mesh m_blockIndicator {};
        HitInfo m_blockIndicatorInfo {};
    };
}
