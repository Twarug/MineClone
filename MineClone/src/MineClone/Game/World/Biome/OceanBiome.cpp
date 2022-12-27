#include "mcpch.h"
#include "OceanBiome.h"

// #include "../Structures/TreeGenerator.h"

namespace mc
{
    OceanBiome::OceanBiome()
        : Biome("Ocean", OceanBiome::GetNoiseParameters(), 50, 100)
    {
    }

    BlockState OceanBiome::GetTopBlock(Rand &rand) const
    {
        return BlockState(Block::GRASS_BLOCK);
    }

    BlockState OceanBiome::GetUnderWaterBlock(Rand &rand) const
    {
        return BlockState(Block::SAND);
    }

    void OceanBiome::MakeTree(Rand &rand, Chunk &chunk, int3 blockPos) const
    {
        // rand.Range(0, 5) < 3 ? makePalmTree(chunk, rand, x, y, z)
        //                           : makeOakTree(chunk, rand, x, y, z);
    }

    NoiseParameters OceanBiome::GetNoiseParameters()
    {
        NoiseParameters heightParams = {
            .octaves = 7,
            .amplitude = 43,
            .smoothness = 55,
            .heightOffset = 0,
            .roughness = 0.50f,
        };
        
        return heightParams;
    }

    BlockState OceanBiome::GetPlant(Rand &rand) const
    {
        return {};//rand.Range(0, 10) > 6 ? Block::Rose : Block::TallGrass;
    }
}