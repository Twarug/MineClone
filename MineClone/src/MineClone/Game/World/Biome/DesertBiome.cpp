#include "mcpch.h"
#include "DesertBiome.h"

// #include "../Structures/TreeGenerator.h"

namespace mc
{
    DesertBiome::DesertBiome()
        : Biome("Desert", DesertBiome::GetNoiseParameters(), 1350, 500)
    {
    }

    BlockState DesertBiome::GetTopBlock(Rand &rand) const
    {
        return BlockState(Block::SAND);
    }

    BlockState DesertBiome::GetUnderWaterBlock(Rand &rand) const
    {
        return BlockState(Block::SAND);
    }

    void DesertBiome::MakeTree(Rand &rand, Chunk &chunk, int3 blockPos) const
    {
        // if (y < WATER_LEVEL + 15) {
        //     if (rand.intInRange(0, 100) > 75) {
        //         MakePalmTree(chunk, rand, x, y, z);
        //     }
        //     else {
        //         MakeCactus(chunk, rand, x, y, z);
        //     }
        // }
        // else {
        //     MakeCactus(chunk, rand, x, y, z);
        // }
    }

    NoiseParameters DesertBiome::GetNoiseParameters()
    {
        NoiseParameters heightParams = {
            .octaves = 9,
            .amplitude = 80,
            .smoothness = 335,
            .heightOffset = -7,
            .roughness = 0.56f,
        };

        return heightParams;
    }

    BlockState DesertBiome::GetPlant(Rand &rand) const
    {
        return BlockState(Block::AIR); // Dead Bush
    }
}