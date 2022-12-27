#pragma once
#include <random>

#include "MineClone/Game/Utils/NoiseGenerator.h"
#include "MineClone/Game/World/ChunkColumn.h"

#include "MineClone/Game/Utils/Random.h"

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
        static void GenerateHeightMap(ChunkColumn& chunkColumn);

    private:
        static bool IsOutsideWorld(int3 chunkID);

        static NoiseGenerator s_noise;
        static Random<std::minstd_rand> s_random;
    };
}
