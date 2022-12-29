#include "mcpch.h"
#include "World.h"

#include "Generator/ChunkGenerator.h"

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

    HitInfo World::RayCast(float3 origin, float3 direction, float distance) {
        //which box of the map we're in
        int3 blockPos = floor(origin);
        

        //length of ray from current position to next x or y-side
        float3 sideDist;

        //length of ray from one x or y-side to next x or y-side
        float3 deltaDist = {
            direction.x == 0 ? 1e30f : std::abs(1.f / direction.x),
            direction.y == 0 ? 1e30f : std::abs(1.f / direction.y),
            direction.z == 0 ? 1e30f : std::abs(1.f / direction.z),
        };
        
        //what direction to step in x or y-direction (either +1 or -1)
        int3 step;

        if (direction.x < 0) {
            step.x = -1;
            sideDist.x = (origin.x - (float)blockPos.x) * deltaDist.x;
        }
        else {
            step.x = 1;
            sideDist.x = ((float)blockPos.x + 1.0f - origin.x) * deltaDist.x;
        }
        
        if (direction.y < 0) {
            step.y = -1;
            sideDist.y = (origin.y - (float)blockPos.y) * deltaDist.y;
        }
        else {
            step.y = 1;
            sideDist.y = ((float)blockPos.y + 1.f - origin.y) * deltaDist.y;
        }
        
        if (direction.z < 0) {
            step.z = -1;
            sideDist.z = (origin.z - (float)blockPos.z) * deltaDist.z;
        }
        else {
            step.z = 1;
            sideDist.z = ((float)blockPos.z + 1.f - origin.z) * deltaDist.z;
        }
        
        //perform DDA
        while (glm::distance(float3(blockPos), origin) < distance)
        {
            float minSideDist = std::min(sideDist.x, std::min(sideDist.y, sideDist.z));
            float3 normal;
            
            //jump to next map square, either in x-direction, or in y-direction
            if (sideDist.x == minSideDist) {
                sideDist.x += deltaDist.x;
                blockPos.x += step.x;
                normal = {(float)-step.x, 0, 0};
            }
            else if(sideDist.y == minSideDist) {
                sideDist.y += deltaDist.y;
                blockPos.y += step.y;
                normal = {0, (float)-step.y, 0};
            } else {
                sideDist.z += deltaDist.z;
                blockPos.z += step.z;
                normal = {0, 0, (float)-step.z};
            }
            
            //Check if ray has hit a wall
            BlockState* blockState = GetBlockState(blockPos);
            if (blockState && blockState->GetBlock() != Block::AIR)
                return {true, blockState, blockPos, normal};
        }
        
        return {};
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
