#include "mcpch.h"
#include "ChunkGenerator.h"

#include <execution>

#include "MineClone/Game/World/World.h"

namespace mc
{
    NoiseGenerator ChunkGenerator::s_noise{0};
    
    void ChunkGenerator::Init(i32 seed) {
        s_noise = NoiseGenerator(seed);
        s_noise.SetParameters({
            .octaves = 5,
            .amplitude = 120,
            .smoothness = 1035,
            .heightOffset = 0,
            .roughness = 0.75f,
        });
    }

    void ChunkGenerator::UpdatePlayer(World& world, int3 currentChunkID) {

        using namespace std::chrono;
        auto start = high_resolution_clock::now();  

        int chunkCount = 0;
        std::for_each(std::execution::unseq, Config::CHUNK_RENDER_PATTERN.begin(), Config::CHUNK_RENDER_PATTERN.end(), [&chunkCount, &world, currentChunkID](int3 relative) {
            int3 chunkID = currentChunkID + relative;
            if(IsOutsideWorld(chunkID))
                return;

            int2 columnID = chunkID.xz;

            ChunkColumn* column = nullptr;
            if(world.m_chunkColumns.contains(columnID))
                column = &world.m_chunkColumns.at(columnID);
            else
                column = &world.m_chunkColumns.emplace(columnID, ChunkColumn(columnID, world)).first->second;

            if(column->GetChunk(chunkID))
                return;
                
            Chunk& chunk = CreateChunk(*column, chunkID);
            GenerateChunk(chunk);
            chunkCount++;
            chunk.UpdateMesh();
            for(const Facing& face : Facing::FACINGS) {
                int3 neighbourChunkID = chunkID + face.directionVec;
                if(Chunk* neighbour = world.GetChunk(neighbourChunkID))
                    neighbour->UpdateMesh();
            }
        });

        
        std::cout << "Generation of " << chunkCount << " chunks for player at chunkID: " << to_string(currentChunkID) << ", took " << duration_cast<milliseconds>(high_resolution_clock::now() - start) << '\n';
    }

    void ChunkGenerator::GenerateChunk(Chunk& chunk) {
        ChunkColumn& column = chunk.m_chunkColumn;

        if(!column.m_hasHeightMap)
            GenerateHeightMap(column);

        for(i32 z = 0; z < Config::CHUNK_SIZE.z; z++)
            for(i32 x = 0; x < Config::CHUNK_SIZE.x; x++) {
                i32 height = column.m_heightMap[x + z * Config::CHUNK_SIZE.x] - chunk.m_id.y * Config::CHUNK_SIZE.y;
                for(i32 y = 0; y < Config::CHUNK_SIZE.y; y++) {
                    const Block* block = &Block::AIR;
                    if(y == height)
                        block = &Block::GRASS_BLOCK;
                    else if(y < height)
                        if (y >= height - 3)
                            block = &Block::DIRT;
                        else
                            block = &Block::STONE;

                    chunk.m_blockStates[Chunk::ToIndex({x, y, z})] = BlockState(*block);
                }
            }
    }

    Chunk& ChunkGenerator::CreateChunk(ChunkColumn& column, int3 chunkID) {
        Scope<Chunk>& chunkPtr = column.m_chunks[chunkID.y];
        chunkPtr.reset(new Chunk(chunkID, column));
        return *chunkPtr;
    }

    void ChunkGenerator::GenerateHeightMap(ChunkColumn& chunkColumn) {
        int2 chunkID = chunkColumn.m_id;
        for(i32 z = 0; z < Config::CHUNK_SIZE.z; z++)
            for(i32 x = 0; x < Config::CHUNK_SIZE.x; x++)
                chunkColumn.m_heightMap[x + z * Config::CHUNK_SIZE.x] = (i32)s_noise.GetHeight(x, z, chunkID.x, chunkID.y);
    }

    bool ChunkGenerator::IsOutsideWorld(int3 chunkID) {
        return chunkID.x < -(i32)Config::WORLD_SIZE.x / 2 || chunkID.x >= (i32)Config::WORLD_SIZE.x / 2 ||
               chunkID.y < 0                              || chunkID.y >= (i32)Config::WORLD_SIZE.y     ||
               chunkID.z < -(i32)Config::WORLD_SIZE.z / 2 || chunkID.z >= (i32)Config::WORLD_SIZE.z / 2;
    }
}
