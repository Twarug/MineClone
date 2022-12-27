#pragma once
#include "MineClone/Game/Utils/Random.h"
#include "MineClone/Game/Utils/NoiseGenerator.h"

#include "MineClone/Game/World/BlockState.h"

namespace mc
{
    struct NoiseParameters;
    using Rand = Random<std::minstd_rand>;

    class Chunk;
    
    class Biome : public RegistryEntry<Biome>
    {
    public:
        // ReSharper disable once CppInconsistentNaming
        inline static Registry<Biome> REGISTRY{};
        
        static const Biome& PLAINS;
        static const Biome& DESERT;
        static const Biome& OCEAN;
        static const Biome& FOREST;
        static const Biome& TAIGA_FOREST;
        
    public:
        Biome(const std::string& name, const NoiseParameters& parameters, i32 treeFreq, i32 plantFreq);
        ~Biome() override = default;

        Biome(const Biome&) = delete;
        Biome& operator=(const Biome&) = delete;
        
        Biome(Biome&&) = delete;
        Biome& operator=(Biome&&) = delete;

    public:
        static void Init(i32 seed);
        
    public:
        virtual BlockState GetPlant(Rand& rand) const = 0;
        virtual BlockState GetTopBlock(Rand& rand) const = 0;
        virtual BlockState GetUnderWaterBlock(Rand& rand) const = 0;
        virtual BlockState GetBeachBlock(Rand& rand) const;
        virtual void MakeTree(Rand &rand, Chunk& chunk, int3 pos) const = 0;

        int GetHeight(int2 chunkPos, int2 chunkID) const;
        int GetTreeFrequency() const noexcept;
        int GetPlantFrequency() const noexcept;

    protected:
        virtual NoiseParameters GetNoiseParameters() = 0;

    private:
        NoiseGenerator m_heightGenerator;
        int m_treeFreq;
        int m_plantFreq;
    };
}
