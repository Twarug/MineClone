#include "mcpch.h"
#include "Biome.h"

#include "DesertBiome.h"
#include "ForestBiome.h"
#include "OceanBiome.h"
#include "PlainsBiome.h"
#include "TaigaForestBiome.h"

namespace mc
{
    const Biome& Biome::PLAINS       = REGISTRY.Register(new PlainsBiome());
    const Biome& Biome::DESERT       = REGISTRY.Register(new DesertBiome());
    const Biome& Biome::OCEAN        = REGISTRY.Register(new OceanBiome());
    const Biome& Biome::FOREST       = REGISTRY.Register(new ForestBiome());
    const Biome& Biome::TAIGA_FOREST = REGISTRY.Register(new TaigaForestBiome());
    
    Biome::Biome(const std::string& name, const NoiseParameters &parameters, i32 treeFreq, i32 plantFreq)
        : RegistryEntry(name), m_treeFreq(treeFreq), m_plantFreq(plantFreq) {
        m_heightGenerator.SetParameters(parameters);
    }

    void Biome::Init(i32 seed) {
        for(Biome* biome : REGISTRY | std::views::values)
            biome->m_heightGenerator.SetSeed(seed);
    }

    BlockState Biome::GetBeachBlock(Rand &rand) const {
        return BlockState(Block::DIRT);
    }

    i32 Biome::GetHeight(int2 chunkPos, int2 chunkID) const
    {
        return (i32)m_heightGenerator.GetHeight(chunkPos.x, chunkPos.y, chunkID.x, chunkID.y);
    }

    i32 Biome::GetTreeFrequency() const noexcept
    {
        return m_treeFreq;
    }

    i32 Biome::GetPlantFrequency() const noexcept
    {
        return m_plantFreq;
    }
}
