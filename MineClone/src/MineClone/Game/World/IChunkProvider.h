#pragma once
#include "MineClone/Config.h"

namespace mc
{
    class Chunk;
    
    class IChunkProvider : public IBlockStateProvider
    {
    protected:
        IChunkProvider() = default;
        ~IChunkProvider() = default;

        IChunkProvider(const IChunkProvider&) = default;
        IChunkProvider& operator=(const IChunkProvider&) = default;
        IChunkProvider(IChunkProvider&&) = default;
        IChunkProvider& operator=(IChunkProvider&&) = default;

    public:
        virtual Chunk* GetChunk(int3 chunkID) = 0;
        virtual const Chunk* GetChunk(int3 chunkID) const = 0;
    };
}
