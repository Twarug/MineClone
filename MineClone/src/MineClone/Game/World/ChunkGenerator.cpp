#include "mcpch.h"
#include "ChunkGenerator.h"

#include <execution>
#include <random>

#include "World.h"

namespace mc
{
    void ChunkGenerator::Init(i32 seed) {
        s_noise = FastNoiseLite(seed);
        // s_noise.SetFractalType(FastNoiseLite::FractalType_PingPong);
        // s_noise.SetFrequency(1.f);        
        // s_noise.SetFractalOctaves(6); 
    }

    void ChunkGenerator::UpdatePlayer(Scope<World>& world, int3 currentChunkID) {

        // std::cout << "New player pos: " << to_string(currentChunkID) << '\n';
        
        std::ranges::for_each(Config::CHUNK_RENDER_PATTERN, [&world, currentChunkID](int3 relative) {
            int3 chunkID = currentChunkID + relative;
            if(IsOutsideWorld(chunkID))
                return;

            int2 columnID = chunkID.xz;

            ChunkColumn* column = nullptr;
            if(world->m_chunkColumns.contains(columnID))
                column = &world->m_chunkColumns.at(columnID);
            else
                column = &world->m_chunkColumns.emplace(columnID, ChunkColumn(columnID, *world)).first->second;

            if(column->GetChunk(chunkID))
                return;
                
            Chunk& chunk = CreateChunk(*column, chunkID);
            GenerateChunk(chunk);
            chunk.UpdateMesh();
            for(const Facing& face : Facing::FACINGS) {
                int3 neighbourChunkID = chunkID + face.directionVec;
                if(Chunk* neighbour = world->GetChunk(neighbourChunkID))
                    neighbour->UpdateMesh();
            }
        });
    }

    void ChunkGenerator::GenerateChunk(Chunk& chunk) {
        ChunkColumn& column = chunk.m_chunkColumn;

        if(!column.m_hasHeightMap)
            GenerateHeightMap(column);

        for(i32 z = 0; z < Config::CHUNK_SIZE.z; z++)
            for(i32 x = 0; x < Config::CHUNK_SIZE.x; x++) {
                i32 height = column.m_heightMap[x + z * Config::CHUNK_SIZE.x] - chunk.m_id.y * Config::CHUNK_SIZE.y;
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
        chunkPtr.reset(new Chunk(chunkID, column));
        return *chunkPtr;
    }

    void ChunkGenerator::GenerateHeightMap(ChunkColumn& chunkColumn) {
        int2 chunkPos = chunkColumn.m_id * Config::CHUNK_SIZE.xz;
        for(i32 z = 0; z < Config::CHUNK_SIZE.z; z++)
            for(i32 x = 0; x < Config::CHUNK_SIZE.x; x++)
                chunkColumn.m_heightMap[x + z * Config::CHUNK_SIZE.x] = (i32)(10.f + s_noise.GetNoise((float)(chunkPos.x + x), (float)(chunkPos.y + z)) * 10.f);
    }

    bool ChunkGenerator::IsOutsideWorld(int3 chunkID) {
        return chunkID.x < -(i32)Config::WORLD_SIZE.x / 2 || chunkID.x >= (i32)Config::WORLD_SIZE.x / 2 ||
               chunkID.y < 0                              || chunkID.y >= (i32)Config::WORLD_SIZE.y     ||
               chunkID.z < -(i32)Config::WORLD_SIZE.z / 2 || chunkID.z >= (i32)Config::WORLD_SIZE.z / 2;
    }
}
