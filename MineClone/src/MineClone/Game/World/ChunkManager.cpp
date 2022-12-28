#include "mcpch.h"
#include "ChunkManager.h"

#include <execution>

#include "Generator/ChunkGenerator.h"


namespace mc
{
    static std::queue<Chunk*> g_generateQueue;
    
    void ChunkManager::Update(World& world) {

        for(int i = 0; i < 1; i++) {
            if(g_generateQueue.empty())
                return;

            Chunk* chunk = g_generateQueue.front();
            g_generateQueue.pop();

            if(!world.GetChunk(chunk->GetID()))
                continue;
        
            ChunkGenerator::GenerateChunk(*chunk);
            chunk->UpdateMesh();
            for(const Facing& face : Facing::FACINGS) {
                int3 neighbourChunkID = chunk->m_id + face.directionVec;
                if(Chunk* neighbour = world.GetChunk(neighbourChunkID))
                    neighbour->UpdateMesh();
            }
        }
    }

    void ChunkManager::UpdatePlayer(World& world, int3 currentChunkID) {

        using namespace std::chrono;
        auto start = high_resolution_clock::now();

        constexpr i32 deleteDistance = (i32)(Config::RENDER_DISTANCE * 1.25f);

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

        int chunkCount = 0;
        std::for_each(std::execution::seq, Config::CHUNK_RENDER_PATTERN.begin(), Config::CHUNK_RENDER_PATTERN.end(), [&chunkCount, &world, currentChunkID](int3 relative) {
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
            g_generateQueue.push(&chunk);
            chunkCount++;
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
