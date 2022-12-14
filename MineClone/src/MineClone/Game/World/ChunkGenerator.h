#pragma once
#include "ChunkColumn.h"

namespace mc
{
    class Chunk;

    class ChunkGenerator
    {
    public:
        static void GenerateChunk(Chunk& chunk);
        static Chunk& CreateChunk(ChunkColumn& column, int3 chunkID);

    private:
        static void GenerateHeightMap(ChunkColumn& chunkColumn);
    };
}
