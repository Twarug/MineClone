#pragma once
#include "Biome.h"

namespace mc
{
    class PlainsBiome : public Biome {
    public:
        PlainsBiome();

        BlockState GetBeachBlock(Rand &rand) const override;
        BlockState GetPlant(Rand &rand) const override;
        BlockState GetTopBlock(Rand &rand) const override;
        BlockState GetUnderWaterBlock(Rand &rand) const override;
        void MakeTree(Rand &rand, Chunk &chunk, int3 blockPos) const override;

    private:
        NoiseParameters GetNoiseParameters() override;
    };
}