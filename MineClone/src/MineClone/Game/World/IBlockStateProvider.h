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
                blockPos.x / Config::CHUNK_SIZE.x - (i32)(blockPos.x < 0),
                blockPos.y / Config::CHUNK_SIZE.y - (i32)(blockPos.y < 0),
                blockPos.z / Config::CHUNK_SIZE.z - (i32)(blockPos.z < 0),
            };
        }

        static int3 ToChunkPos(int3 blockPos) {
            return {
                std::abs(blockPos.x % Config::CHUNK_SIZE.x + (blockPos.x < 0) * Config::CHUNK_SIZE.x),
                std::abs(blockPos.y % Config::CHUNK_SIZE.y + (blockPos.y < 0) * Config::CHUNK_SIZE.y),
                std::abs(blockPos.z % Config::CHUNK_SIZE.z + (blockPos.z < 0) * Config::CHUNK_SIZE.z),
            };
        }
    };
}
