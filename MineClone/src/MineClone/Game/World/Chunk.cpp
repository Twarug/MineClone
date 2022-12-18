#include "mcpch.h"
#include "Chunk.h"

#include <glm/ext/matrix_transform.hpp>

#include "ChunkColumn.h"
#include "MineClone/Game/Utils/Facing.h"

namespace mc
{
    Chunk::Chunk(ChunkColumn& chunkColumn, int3 pos)
        : m_chunkColumn(chunkColumn), m_pos(pos), m_transform(translate(Mat4{1}, float3(pos) * float3(Config::CHUNK_SIZE))) {}

    void Chunk::Tick(World& world) {
        int3 pos = {0, 0, 0};
        for(; pos.z < Config::CHUNK_SIZE.z; pos.z++)
            for(; pos.y < Config::CHUNK_SIZE.y; pos.y++)
                for(; pos.x < Config::CHUNK_SIZE.x; pos.x++)
                {
                    BlockState& blockState = m_blockStates[ToIndex(pos)];
                    blockState.GetBlock().Tick(world, pos, blockState);
                }
    }
    
    void Chunk::UpdateMesh() {
        std::vector<Vertex3D> vertices;
        std::vector<u32> indices;

        for(u64 z = 0; z < Config::CHUNK_SIZE.z; z++)
            for(u64 y = 0; y < Config::CHUNK_SIZE.y; y++)
                for(u64 x = 0; x < Config::CHUNK_SIZE.x; x++) {
                    int3 chunkPos = {x, y, z};
                    BlockState& current = m_blockStates[ToIndex(chunkPos)];

                    if(current.GetBlock().IsTransparent())
                        continue;

                    float3 color = current.GetBlock().GetColor();
                    
                    for(u64 i = 0; i < Facing::FACINGS.size(); i++) {
                        int3 neighbourPos = chunkPos + Facing::FACINGS[i].directionVec;
                        
                        BlockState* neighbour = nullptr;
                        if(neighbourPos.x < 0 || neighbourPos.x >= Config::CHUNK_SIZE.x ||
                           neighbourPos.y < 0 || neighbourPos.y >= Config::CHUNK_SIZE.y ||
                           neighbourPos.z < 0 || neighbourPos.z >= Config::CHUNK_SIZE.z)
                            neighbour = m_chunkColumn.GetBlockState(m_pos * Config::CHUNK_SIZE + neighbourPos);
                        else
                            neighbour = &m_blockStates[ToIndex(neighbourPos)];

                        if(neighbour && neighbour->GetBlock().IsTransparent() ||
                           !neighbour && chunkPos.y == Config::CHUNK_SIZE.y - 1) {
                            auto face = Config::VERTICES[i];
                            u32 firstVertexIndex = (u32)vertices.size();
                            
                            for(Vertex3D v : face) {
                                v.pos += chunkPos;
                                v.color = color;
                                vertices.push_back(v);
                            }
                            
                            indices.push_back(firstVertexIndex + 0);
                            indices.push_back(firstVertexIndex + 3);
                            indices.push_back(firstVertexIndex + 1);
                            indices.push_back(firstVertexIndex + 0);
                            indices.push_back(firstVertexIndex + 2);
                            indices.push_back(firstVertexIndex + 3);
                        }                            
                    }
                }

        m_mesh.SetIndices(indices);
        m_mesh.SetVertices(std::span(vertices));
    }

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

    u64 Chunk::ToIndex(int3 chunkPos) {
        return (u64)chunkPos.x + (u64)chunkPos.y * Config::CHUNK_SIZE.x + (u64)chunkPos.z * Config::CHUNK_SIZE.x * Config::CHUNK_SIZE.y;
    }
}
