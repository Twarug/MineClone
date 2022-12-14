#pragma once
#include "BlockState.h"
#include "MineClone/Config.h"

namespace mc
{
    class IBlockStateProvider
    {
    protected:
        IBlockStateProvider() = default;
        ~IBlockStateProvider() = default;

        IBlockStateProvider(const IBlockStateProvider&) = default;
        IBlockStateProvider& operator=(const IBlockStateProvider&) = default;
        IBlockStateProvider(IBlockStateProvider&&) = default;
        IBlockStateProvider& operator=(IBlockStateProvider&&) = default;

    public:
        virtual BlockState* GetBlockState(int3 blockPos) = 0;
        virtual const BlockState* GetBlockState(int3 blockPos) const = 0;

        virtual void SetBlockState(int3 blockPos, const BlockState& blockState) = 0;

    protected:
        static int3 ToChunkID(int3 blockPos) {
            return {
                blockPos.x / Config::CHUNK_SIZE.x,
                blockPos.y / Config::CHUNK_SIZE.y,
                blockPos.z / Config::CHUNK_SIZE.z,
            };
        }

        static int3 ToChunkPos(int3 blockPos) {
            return {
                blockPos.x % Config::CHUNK_SIZE.x,
                blockPos.y % Config::CHUNK_SIZE.y,
                blockPos.z % Config::CHUNK_SIZE.z,
            };
        }
    };
}
