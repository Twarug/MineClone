#include "mcpch.h"
#include "BlockState.h"

namespace mc
{
    BlockState::BlockState()
        : m_block(&Block::AIR) {}

    BlockState::BlockState(Identifier blockId)
        : m_block(&Block::REGISTRY.GetByID(blockId)) {}

    BlockState::BlockState(const Block& block)
        : m_block(&block) {}
}
