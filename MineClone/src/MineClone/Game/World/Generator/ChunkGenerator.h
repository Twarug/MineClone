#pragma once
#include "MineClone/Game/World/ChunkColumn.h"

#include "FastNoiseLite.h"

namespace mc
{
    class Chunk;

    class ChunkGenerator
    {
    public:
        static void Init(i32 seed);
        
        static void UpdatePlayer(World& world, int3 currentChunkID);

        static void GenerateChunk(Chunk& chunk);
        static Chunk& CreateChunk(ChunkColumn& column, int3 chunkID);

        static void GenerateHeightMap(ChunkColumn& chunkColumn);

    private:
        static bool IsOutsideWorld(int3 chunkID);

        inline static FastNoiseLite s_noise{};
    };
}
