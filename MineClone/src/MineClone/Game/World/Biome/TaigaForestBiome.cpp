#include "mcpch.h"
#include "TaigaForestBiome.h"

// #include "../Structures/TreeGenerator.h"

namespace mc
{
    TaigaForestBiome::TaigaForestBiome()
        : Biome("Taiga", TaigaForestBiome::GetNoiseParameters(), 55, 75) {}
    

    BlockState TaigaForestBiome::GetTopBlock(Rand &rand) const
    {
        return rand.Range(0, 10) < 8 ? BlockState(Block::GRASS_BLOCK) : BlockState(Block::DIRT);
    }

    BlockState TaigaForestBiome::GetUnderWaterBlock(Rand &rand) const
    {
        return rand.Range(0, 10) > 8 ? BlockState(Block::DIRT) : BlockState(Block::SAND);
    }

    void TaigaForestBiome::MakeTree(Rand &rand, Chunk &chunk, int3 blockPos) const
    {
        // makeOakTree(chunk, rand, x, y, z);
    }

    NoiseParameters TaigaForestBiome::GetNoiseParameters()
    {
        NoiseParameters heightParams = {
            .octaves = 5,
            .amplitude = 100,
            .smoothness = 195,
            .heightOffset = -30,
            .roughness = 0.52f,
        };
        
        return heightParams;
    }

    BlockState TaigaForestBiome::GetPlant(Rand &rand) const
    {
        return {};//Block::TallGrass;
    }
}