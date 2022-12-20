#pragma once
#include "ChunkColumn.h"

namespace mc
{
    class Chunk;

    class ChunkGenerator
    {
    public:
        static void UpdatePlayer(Scope<World>& world, int3 currentChunkID);

        static void GenerateChunk(Chunk& chunk);
        static Chunk& CreateChunk(ChunkColumn& column, int3 chunkID);

        static void GenerateHeightMap(ChunkColumn& chunkColumn);

    private:
        static bool IsOutsideWorld(int3 chunkID);
    };
}
