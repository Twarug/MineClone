#include "mcpch.h"
#include "ChunkGenerator.h"

#include <random>

namespace mc
{    
    void ChunkGenerator::GenerateChunk(Chunk& chunk) {
        ChunkColumn& column = chunk.m_chunkColumn;

        if(!column.m_hasHeightMap)
            GenerateHeightMap(column);

        for(i32 z = 0; z < Config::CHUNK_SIZE.z; z++)
            for(i32 x = 0; x < Config::CHUNK_SIZE.x; x++) {
                i32 height = column.m_heightMap[x + z * Config::CHUNK_SIZE.x] - chunk.m_pos.y * Config::CHUNK_SIZE.y;
                for(i32 y = 0; y < Config::CHUNK_SIZE.y; y++) {
                    if(y == height)
                        chunk.m_blockStates[Chunk::ToIndex({x, y, z})] = BlockState(Block::DIRT);
                    else if(y < height)
                        chunk.m_blockStates[Chunk::ToIndex({x, y, z})] = BlockState(Block::STONE);
                }
            }
    }

    Chunk& ChunkGenerator::CreateChunk(ChunkColumn& column, int3 chunkID) {
        Scope<Chunk>& chunkPtr = column.m_chunks[chunkID.y];
        chunkPtr.reset(new Chunk(column, chunkID));
        return *chunkPtr;
    }

    void ChunkGenerator::GenerateHeightMap(ChunkColumn& chunkColumn) {
        int2 chunkPos = chunkColumn.m_pos * Config::CHUNK_SIZE.xz;
        for(i32 z = 0; z < Config::CHUNK_SIZE.z; z++)
            for(i32 x = 0; x < Config::CHUNK_SIZE.x; x++)
                chunkColumn.m_heightMap[x + z * Config::CHUNK_SIZE.x] = 10 + (i32)(sin((x + chunkPos.x) / 3.0 + (z  + chunkPos.y) / 8.0) * 1.5);
    }
}
