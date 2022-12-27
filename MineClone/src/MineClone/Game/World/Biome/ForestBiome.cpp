#include "mcpch.h"
#include "ForestBiome.h"

// #include "../Structures/TreeGenerator.h"

namespace mc
{
    ForestBiome::ForestBiome()
        : Biome("Forest", ForestBiome::GetNoiseParameters(), 60, 80)
    {
    }

    BlockState ForestBiome::GetTopBlock(Rand &rand) const
    {
        return BlockState(Block::GRASS_BLOCK);
    }

    BlockState ForestBiome::GetUnderWaterBlock(Rand &rand) const
    {
        return rand.Range(0, 10) > 9 ? BlockState(Block::SAND) : BlockState(Block::DIRT);
    }

    void ForestBiome::MakeTree(Rand &rand, Chunk &chunk, int3 blockPos) const
    {
        // makeOakTree(chunk, rand, x, y, z);
    }

    NoiseParameters ForestBiome::GetNoiseParameters()
    {
        NoiseParameters heightParams = {
            .octaves = 5,
            .amplitude = 100,
            .smoothness = 195,
            .heightOffset = -32,
            .roughness = 0.52f,
        };

        return heightParams;
    }

    BlockState ForestBiome::GetPlant(Rand &rand) const
    {
        return {};//rand.Range(0, 10) > 8 ? BlockState(Block::FLOWER) : BlockState(Block::TALL_GRASS);
    }
}