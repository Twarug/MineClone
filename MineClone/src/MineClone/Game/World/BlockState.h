#pragma once
#include "MineClone/Game/Block/Block.h"

namespace mc
{
    class BlockState
    {
    public:
        BlockState();
        explicit BlockState(Identifier blockId);
        explicit BlockState(const Block& block);

    public:
        ~BlockState() = default;
        BlockState(const BlockState& other) = default;
        BlockState(BlockState&& other) noexcept = default;
        BlockState& operator=(const BlockState& other) = default;
        BlockState& operator=(BlockState&& other) noexcept = default;

    public:
        const Block& GetBlock() const { return *m_block; }

    private:
        const Block* m_block;

        //todo: nbt
    };
}
