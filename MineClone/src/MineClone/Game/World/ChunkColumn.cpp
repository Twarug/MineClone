#include "mcpch.h"
#include "ChunkColumn.h"

#include "World.h"

namespace mc
{
    ChunkColumn::ChunkColumn(World& world, int2 pos)
        : m_world(world), m_pos(pos) {}

    void ChunkColumn::Tick() {
        for(Scope<Chunk>& chunk : m_chunks)
            if(chunk)
                chunk->Tick(m_world);
    }
    
    void ChunkColumn::UpdateMesh() {
        for(Scope<Chunk>& chunk : m_chunks)
            if(chunk)
                chunk->UpdateMesh();
    }

    void ChunkColumn::Render() {
        for(Scope<Chunk>& chunk : m_chunks)
            if(chunk)
                chunk->Render();
    }

    BlockState* ChunkColumn::GetBlockState(int3 blockPos) {
        if(blockPos.y >= (i32)(Config::WORLD_SIZE.y * Config::CHUNK_SIZE.y))
            return nullptr;

        if(blockPos.y < 0)
            return nullptr;        
        
        int3 chunkID = ToChunkID(blockPos);
        int2 columnID = int2(chunkID.xz);
        
        if(columnID != m_pos)
            return m_world.GetBlockState(blockPos);

        if(const Scope<Chunk>& chunk = m_chunks[chunkID.y])
            return chunk->GetBlockState(blockPos);

        return nullptr;
    }

    const BlockState* ChunkColumn::GetBlockState(int3 blockPos) const {
        if(blockPos.y >= (i32)(Config::WORLD_SIZE.y * Config::CHUNK_SIZE.y))
            return nullptr;

        if(blockPos.y < 0)
            return nullptr;
        
        int3 chunkID = ToChunkID(blockPos);
        int2 columnID = int2(chunkID.xz);

        if(columnID != m_pos)
            return m_world.GetBlockState(blockPos);

        if(const Scope<Chunk>& chunk = m_chunks[chunkID.y])
            return chunk->GetBlockState(blockPos);

        return nullptr;
    }

    void ChunkColumn::SetBlockState(int3 blockPos, const BlockState& blockState) {
        int3 chunkID = ToChunkID(blockPos);
        int2 columnID = int2(chunkID.xz);

        if(columnID != m_pos) {
            m_world.SetBlockState(blockPos, blockState);
            return;
        }

        if(const Scope<Chunk>& chunk = m_chunks[chunkID.y])
            chunk->SetBlockState(blockPos, blockState);
        else
            std::cout << "Trying to set block " << blockState.GetBlock().GetName() << " on non existing chunk\n";
    }
}
