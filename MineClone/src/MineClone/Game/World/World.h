#pragma once
#include "ChunkColumn.h"
#include "IChunkProvider.h"

namespace mc
{
    class World final : public IChunkProvider
    {
    public:
        World();
        virtual ~World() = default;

    public:
        World(const World& other) = default;
        World(World&& other) noexcept = default;
        World& operator=(const World& other) = default;
        World& operator=(World&& other) noexcept = default;

    public:
        void Tick();

        void Render();

    public:
        Chunk* GetChunk(int3 chunkID) override;
        const Chunk* GetChunk(int3 chunkID) const override;
        
        BlockState* GetBlockState(int3 blockPos) override;
        const BlockState* GetBlockState(int3 blockPos) const override;
        void SetBlockState(int3 blockPos, const BlockState& blockState) override;

    private:
        std::unordered_map<int2, ChunkColumn> m_chunkColumns;
    };
}
