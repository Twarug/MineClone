#pragma once

#include "MineClone/Game/Utils/NoiseGenerator.h"
#include "MineClone/Game/World/ChunkColumn.h"

namespace mc
{
    class Chunk;

    class ChunkGenerator
    {
    public:
        static void Init(i32 seed);
        
        static void UpdatePlayer(World& world, int3 currentChunkID);

        static Chunk& CreateChunk(ChunkColumn& column, int3 chunkID);

        static void GenerateChunk(Chunk& chunk);

    private:
        static void SetBlock(Chunk& chunk, int3 pos, const BlockState& blockState);
        
        static void GenerateBiomeMap(ChunkColumn& chunkColumn);
        
        static void GenerateHeightMap(ChunkColumn& chunkColumn);
        static i32 GenerateHeightMapIn(ChunkColumn& chunkColumn, i32 xMin, i32 zMin, i32 xMax, i32 zMax);
        
        static bool IsOutsideWorld(int3 chunkID);

        static NoiseGenerator s_biomeNoise;
    };
}
