#pragma once
#include "Chunk.h"
#include "MineClone/Config.h"

namespace mc
{
    class World;

    class ChunkColumn final : public IBlockStateProvider
    {
    public:
        explicit ChunkColumn(World& world, int2 pos);
        virtual ~ChunkColumn() = default;

        ChunkColumn(const ChunkColumn& other) = delete;
        ChunkColumn(ChunkColumn&& other) noexcept = default;
        ChunkColumn& operator=(const ChunkColumn& other) = delete;
        ChunkColumn& operator=(ChunkColumn&& other) noexcept = delete;

    public:
        void Update();
        
        void UpdateMesh();
        void Render();

    public:
        const auto& GetChunks() const { return m_chunks; }

    public:
        BlockState* GetBlockState(int3 blockPos) override;
        const BlockState* GetBlockState(int3 blockPos) const override;
        void SetBlockState(int3 blockPos, const BlockState& blockState) override;

    private:
        World& m_world;

        bool m_hasHeightMap = false;
        std::array<i32, (u64)Config::CHUNK_SIZE.x * Config::CHUNK_SIZE.z> m_heightMap;

        std::array<Scope<Chunk>, Config::WORLD_SIZE.y> m_chunks;

        int2 m_pos;
        
        friend class ChunkGenerator;
    };
}
