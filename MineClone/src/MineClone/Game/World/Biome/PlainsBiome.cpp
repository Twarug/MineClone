#include "mcpch.h"
#include "PlainsBiome.h"

// #include "../Structures/TreeGenerator.h"

namespace mc
{
    PlainsBiome::PlainsBiome()
        : Biome("Plains", PlainsBiome::GetNoiseParameters(), 1000, 20)
    {
    }

    BlockState PlainsBiome::GetTopBlock(Rand &rand) const
    {
        return BlockState(Block::GRASS_BLOCK);
    }

    BlockState PlainsBiome::GetUnderWaterBlock(Rand &rand) const
    {
        return rand.Range(0, 10) > 8 ? BlockState(Block::DIRT) : BlockState(Block::SAND);
    }

    BlockState PlainsBiome::GetBeachBlock(Rand &rand) const
    {
        return rand.Range(0, 10) > 2 ? BlockState(Block::GRASS_BLOCK) : BlockState(Block::DIRT);
    }

    void PlainsBiome::MakeTree(Rand &rand, Chunk &chunk, int3 blockPos) const
    {
        // makeOakTree(chunk, rand, x, y, z);
    }

    NoiseParameters PlainsBiome::GetNoiseParameters()
    {
        NoiseParameters heightParams = {
            .octaves = 9,
            .amplitude = 85,
            .smoothness = 235,
            .heightOffset = -20,
            .roughness = 0.51f,
        };
        
        return heightParams;
    }

    BlockState PlainsBiome::GetPlant(Rand &rand) const
    {
        return {};//rand.Range(0, 10) > 6 ? Block::Rose : Block::TallGrass;
    }
}