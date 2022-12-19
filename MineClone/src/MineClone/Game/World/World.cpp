#include "mcpch.h"
#include "World.h"

#include "ChunkGenerator.h"

namespace mc
{
    World::World() {

        for(u64 z = 0; z < Config::WORLD_SIZE.z; z++)
            for(u64 x = 0; x < Config::WORLD_SIZE.x; x++) {
                int2 columnPos = {x, z};
                ChunkColumn& column = m_chunkColumns.emplace(columnPos, ChunkColumn(columnPos, *this)).first->second;

                for(u64 y = 0; y < Config::WORLD_SIZE.y; y++)
                    ChunkGenerator::CreateChunk(column, {x, y, z});
            }

        for(ChunkColumn& column : m_chunkColumns | std::views::values)
            for(const Scope<Chunk>& chunk : column.GetChunks())
                if(chunk)
                    ChunkGenerator::GenerateChunk(*chunk);
        
        for(ChunkColumn& column : m_chunkColumns | std::views::values)
            for(const Scope<Chunk>& chunk : column.GetChunks())
                if(chunk)
                    chunk->UpdateMesh();
            // column.UpdateMesh();
    }

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

        std::cout << "Trying to set block " << blockState.GetBlock().GetName() << " in non existing chunk column.\n";
    }
}
