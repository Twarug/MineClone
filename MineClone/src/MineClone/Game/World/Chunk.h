#pragma once
#include "BlockState.h"
#include "IBlockStateProvider.h"
#include "MineClone/Config.h"
#include "MineClone/Core/Renderer/Mesh.h"

namespace mc
{
    class ChunkColumn;
}

namespace mc
{
    class Chunk final : public IBlockStateProvider
    {
    public:
        explicit Chunk(int3 id, ChunkColumn& chunkColumn);
        virtual ~Chunk() = default;

        Chunk(const Chunk& other) = delete;
        Chunk(Chunk&& other) noexcept = delete;
        Chunk& operator=(const Chunk& other) = delete;
        Chunk& operator=(Chunk&& other) noexcept = delete;

    public:
        void Tick(World& world);

        void UpdateMesh();        
        void Render() const;

    public:
        int3 GetID() const { return m_id; }
 
        BlockState* GetBlockState(int3 blockPos) override;
        const BlockState* GetBlockState(int3 blockPos) const override;

        void SetBlockState(int3 blockPos, const BlockState& blockState) override;

    private:
        static u64 ToIndex(int3 chunkPos);
            
    private:
        int3 m_id;
        ChunkColumn& m_chunkColumn;
        
        std::array<BlockState, (u64)(Config::CHUNK_SIZE.x * Config::CHUNK_SIZE.y * Config::CHUNK_SIZE.z)> m_blockStates;
        
        Mesh m_mesh;
        Mat4 m_transform{};

        friend class ChunkGenerator;
    };
}
