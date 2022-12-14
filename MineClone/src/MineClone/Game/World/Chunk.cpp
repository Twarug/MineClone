#include "mcpch.h"
#include "Chunk.h"

#include <glm/ext/matrix_transform.hpp>

#include "ChunkColumn.h"

namespace mc
{
    u64 ToIndex(int3 chunkPos);

    Chunk::Chunk(ChunkColumn& chunkColumn, int3 pos)
        : m_chunkColumn(chunkColumn), m_pos(pos), m_transform(translate(Mat4{1}, float3(pos) * float3(Config::CHUNK_SIZE))) {}

    void Chunk::Render() const {
        m_mesh.Render(m_transform);
    }

    BlockState* Chunk::GetBlockState(int3 blockPos) {
        int3 chunkID = ToChunkID(blockPos);

        if(chunkID != m_pos)
            return m_chunkColumn.GetBlockState(blockPos);

        return &m_blockStates[ToIndex(ToChunkPos(blockPos))];
    }

    const BlockState* Chunk::GetBlockState(int3 blockPos) const {
        int3 chunkID = ToChunkID(blockPos);

        if(chunkID != m_pos)
            return m_chunkColumn.GetBlockState(blockPos);

        return &m_blockStates[ToIndex(ToChunkPos(blockPos))];
    }

    void Chunk::SetBlockState(int3 blockPos, const BlockState& blockState) {
        int3 chunkID = ToChunkID(blockPos);

        if(chunkID != m_pos) {
            m_chunkColumn.SetBlockState(blockPos, blockState);
            return;
        }

        m_blockStates[ToIndex(ToChunkPos(blockPos))] = blockState;
    }


    u64 ToIndex(int3 chunkPos) {
        return chunkPos.x + chunkPos.y * Config::CHUNK_SIZE.x + chunkPos.z * Config::CHUNK_SIZE.x * Config::CHUNK_SIZE.y;
    }
}
