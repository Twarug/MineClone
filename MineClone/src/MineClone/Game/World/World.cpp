#include "mcpch.h"
#include "World.h"

#include "ChunkGenerator.h"

namespace mc
{
    World::World() {}

    void World::Tick() {
        for(auto& chunkColumn : m_chunkColumns | std::views::values)
            chunkColumn.Tick();
    }

    void World::Render() {
        for(auto& chunkColumn : m_chunkColumns | std::views::values)
            chunkColumn.Render();
    }

    Chunk* World::GetChunk(int3 chunkID) {
        int2 columnID = chunkID.xz;
        if(!m_chunkColumns.contains(columnID))
            return nullptr;
        return m_chunkColumns.at(columnID).GetChunk(chunkID);
    }

    const Chunk* World::GetChunk(int3 chunkID) const {
        int2 columnID = chunkID.xz;
        if(!m_chunkColumns.contains(columnID))
            return nullptr;
        return m_chunkColumns.at(columnID).GetChunk(chunkID);
    }

    BlockState* World::GetBlockState(int3 blockPos) {
        int3 chunkID = ToChunkID(blockPos);
        int2 columnID = int2(chunkID.xz);

        if(m_chunkColumns.contains(columnID))
            return m_chunkColumns.at(columnID).GetBlockState(blockPos);

        return nullptr;
    }

    const BlockState* World::GetBlockState(int3 blockPos) const {
        int3 chunkID = ToChunkID(blockPos);
        int2 columnID = int2(chunkID.xz);

        if(m_chunkColumns.contains(columnID))
            return m_chunkColumns.at(columnID).GetBlockState(blockPos);

        return nullptr;
    }

    void World::SetBlockState(int3 blockPos, const BlockState& blockState) {
        int3 chunkID = ToChunkID(blockPos);
        int2 columnID = int2(chunkID.xz);

        if(m_chunkColumns.contains(columnID)) {
            m_chunkColumns.at(columnID).SetBlockState(blockPos, blockState);
            return;
        }

        std::cout << "Trying to set block " << blockState.GetBlock().GetName() << " in non existing chunk column " << to_string(blockPos) << '\n';
    }
}
