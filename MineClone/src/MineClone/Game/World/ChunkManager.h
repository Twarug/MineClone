#pragma once
#include "World.h"

namespace mc
{
    class ChunkManager
    {
    public:
        static void UpdatePlayer(World& world, int3 currentChunkID);

        static Chunk& CreateChunk(ChunkColumn& column, int3 chunkID);
        static bool IsOutsideWorld(int3 chunkID);
    };
}
