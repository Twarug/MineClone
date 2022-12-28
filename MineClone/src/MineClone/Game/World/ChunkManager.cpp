#include "mcpch.h"
#include "ChunkManager.h"

#include <execution>

#include "Generator/ChunkGenerator.h"


namespace mc
{
    void ChunkManager::UpdatePlayer(World& world, int3 currentChunkID) {

        using namespace std::chrono;
        auto start = high_resolution_clock::now();

        constexpr i32 deleteDistance = (i32)(Config::RENDER_DISTANCE * 2.f);

        auto idsView = world.m_chunkColumns | std::views::keys;
        std::vector<int2> columnsIDs{std::begin(idsView), std::end(idsView)};
        for(int2 columnID : columnsIDs) {
            int2 columnPosDelta = abs(columnID - int2{currentChunkID.xz});

            if(columnPosDelta.x > deleteDistance || columnPosDelta.y > deleteDistance) {
                world.m_chunkColumns.erase(columnID);
                continue;
            }

            for(i32 y = 0; y < (i32)Config::WORLD_SIZE.y; y++) {
                if(abs(y - currentChunkID.y) <= deleteDistance)
                    continue;
                
                Scope<Chunk>& chunk = world.m_chunkColumns.at(columnID).m_chunks.at(y);

                if(!chunk)
                    continue;
                
                chunk.reset();
            }
        }
        
            for(i32 z = -deleteDistance; z < deleteDistance; z++) {
                for(i32 x = -deleteDistance; x < deleteDistance; x++) {

                    int2 columnID = currentChunkID.xz + int2{x, z};
                
                    if(!world.m_chunkColumns.contains(columnID))
                        continue;
                    
                    ChunkColumn& column = world.m_chunkColumns.at(columnID);

                    for(i32 dy = -deleteDistance; dy < deleteDistance; dy++) {
                        if(dy >= -(i32)Config::RENDER_DISTANCE && dy < (i32)Config::RENDER_DISTANCE &&
                           x  >= -(i32)Config::RENDER_DISTANCE && x  < (i32)Config::RENDER_DISTANCE &&
                           z  >= -(i32)Config::RENDER_DISTANCE && z  < (i32)Config::RENDER_DISTANCE) {
                        
                            dy = Config::RENDER_DISTANCE;
                            continue;
                           }
                
                        int y = dy + currentChunkID.y;
                
                        if(y < 0 || y >= (i32)Config::WORLD_SIZE.y)
                            continue;
                    
                        Scope<Chunk>& chunk = column.m_chunks.at(y);
                
                        if(!chunk)
                            continue;

                        std::cout << to_string(int3{x, y, z}) << '\n';
                        chunk.reset();
                    }

                    // world.m_chunkColumns.erase(columnID);
                }
            }

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
