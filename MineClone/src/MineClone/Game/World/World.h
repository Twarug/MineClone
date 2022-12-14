#pragma once
#include "ChunkColumn.h"

namespace mc
{
    class World final : public IBlockStateProvider
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
        void Update();

        void Render();

    public:
        BlockState* GetBlockState(int3 blockPos) override;
        const BlockState* GetBlockState(int3 blockPos) const override;
        void SetBlockState(int3 blockPos, const BlockState& blockState) override;

    private:
        std::unordered_map<int2, ChunkColumn> m_chunkColumns;
    };
}
