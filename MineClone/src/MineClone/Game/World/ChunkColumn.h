#pragma once
#include "Chunk.h"
#include "IChunkProvider.h"
#include "MineClone/Config.h"

namespace mc
{
    class World;

    class ChunkColumn final : public IChunkProvider
    {
    public:
        explicit ChunkColumn(int2 id, World& world);
        virtual ~ChunkColumn() = default;

        ChunkColumn(const ChunkColumn& other) = delete;
        ChunkColumn(ChunkColumn&& other) noexcept = default;
        ChunkColumn& operator=(const ChunkColumn& other) = delete;
        ChunkColumn& operator=(ChunkColumn&& other) noexcept = delete;

    public:
        void Tick();
        
        void UpdateMesh();
        void Render();

    public:
        Chunk* GetChunk(int3 chunkID) override;
        const Chunk* GetChunk(int3 chunkID) const override;
        
        const auto& GetChunks() const { return m_chunks; }

    public:
        BlockState* GetBlockState(int3 blockPos) override;
        const BlockState* GetBlockState(int3 blockPos) const override;
        void SetBlockState(int3 blockPos, const BlockState& blockState) override;

    private:
        int2 m_id;
        World& m_world;

        bool m_hasHeightMap = false;
        std::array<i32, (u64)Config::CHUNK_SIZE.x * Config::CHUNK_SIZE.z> m_heightMap{};

        std::array<Scope<Chunk>, Config::WORLD_SIZE.y> m_chunks{};

        
        friend class ChunkGenerator;
    };
}
