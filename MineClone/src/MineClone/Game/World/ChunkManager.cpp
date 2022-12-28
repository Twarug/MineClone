#include "mcpch.h"
#include "ChunkManager.h"

#include <execution>

#include "Generator/ChunkGenerator.h"


namespace mc
{
    void ChunkManager::UpdatePlayer(World& world, int3 currentChunkID) {

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
            ChunkGenerator::GenerateChunk(chunk);
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

    Chunk& ChunkManager::CreateChunk(ChunkColumn& column, int3 chunkID) {
        Scope<Chunk>& chunkPtr = column.m_chunks[chunkID.y];
        chunkPtr.reset(new Chunk(chunkID, column));
        return *chunkPtr;
    }

    bool ChunkManager::IsOutsideWorld(int3 chunkID) {
        return chunkID.x < -(i32)Config::WORLD_SIZE.x / 2 || chunkID.x >= (i32)Config::WORLD_SIZE.x / 2 ||
               chunkID.y < 0                              || chunkID.y >= (i32)Config::WORLD_SIZE.y     ||
               chunkID.z < -(i32)Config::WORLD_SIZE.z / 2 || chunkID.z >= (i32)Config::WORLD_SIZE.z / 2;
    }
}
