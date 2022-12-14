#pragma once
#include "BlockState.h"
#include "BlockStateProvider.h"
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
        explicit Chunk(ChunkColumn& chunkColumn, int3 pos);
        virtual ~Chunk() = default;

        Chunk(const Chunk& other) = delete;
        Chunk(Chunk&& other) noexcept = delete;
        Chunk& operator=(const Chunk& other) = delete;
        Chunk& operator=(Chunk&& other) noexcept = delete;

    public:
        void Render() const;

    public:
        BlockState* GetBlockState(int3 blockPos) override;
        const BlockState* GetBlockState(int3 blockPos) const override;

        void SetBlockState(int3 blockPos, const BlockState& blockState) override;

    private:
        ChunkColumn& m_chunkColumn;
        std::array<BlockState, Config::WORLD_SIZE.x * Config::WORLD_SIZE.y * Config::WORLD_SIZE.z> m_blockStates;

        int3 m_pos;

        Mesh m_mesh;
        Mat4 m_transform{};
    };
}
