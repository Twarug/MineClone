#include "mcpch.h"
#include "ChunkGenerator.h"

#include "MineClone/Game/World/World.h"
#include "MineClone/Game/World/Biome/Biome.h"

namespace mc
{
    NoiseGenerator ChunkGenerator::s_biomeNoise{};

    float SmoothInterpolation(float bottomLeft, float topLeft, float bottomRight, float topRight,
                              float xMin, float xMax, float zMin, float zMax,
                              float x, float z);
    
    void ChunkGenerator::Init(i32 seed) {
        s_biomeNoise = NoiseGenerator(seed);
        s_biomeNoise.SetParameters({
            .octaves = 5,
            .amplitude = 120,
            .smoothness = 1035,
            .heightOffset = 0,
            .roughness = 0.75f,
        });
        Biome::Init(seed);
    }

    void ChunkGenerator::GenerateChunk(Chunk& chunk) {
        ChunkColumn& column = chunk.m_chunkColumn;
        int2 columnID = column.m_id;
        Random<std::minstd_rand> random{(columnID.x ^ columnID.y) << 2};

        if(!column.m_hasHeightMap) {
            GenerateBiomeMap(column);
            GenerateHeightMap(column);
        }
        
        int3 blockPos = chunk.m_id * Config::CHUNK_SIZE;
        if(blockPos.y > column.m_maxHeight)
            return;
        
        for(i32 z = 0; z < Config::CHUNK_SIZE.z; z++)
            for(i32 x = 0; x < Config::CHUNK_SIZE.x; x++) {
                i32 index = x + z * Config::CHUNK_SIZE.x;
                i32 height = column.m_heightMap[index];
                const Biome* biome = column.m_biomeMap[index];
                
                for(i32 dy = 0; dy < Config::CHUNK_SIZE.y; dy++) {

                    i32 y = blockPos.y + dy;
                    
                    BlockState blockState;
                    if (y > height) {
                        if (y <= Config::SEA_LEVEL)
                            blockState = {}; //{Block::WATER};
                        else
                            continue;
                    }
                    else if(y == height) {
                        if (y >= Config::SEA_LEVEL) {
                            if (y < Config::SEA_LEVEL + 4)
                                // Beach
                                blockState = biome->GetBeachBlock(random);
                            else {
                                // if (random.Range(0, biome->GetTreeFrequency()) == 5)
                                //     trees.emplace_back(x, dy + 1, z);
                                
                                if (random.Range(0, biome->GetPlantFrequency()) == 5) {
                                    BlockState plant = biome->GetPlant(random);
                                    if(dy < 15)
                                        SetBlock(chunk, {x, dy + 1, z}, plant);
                                    else
                                        if(Scope<Chunk>& ch = column.m_chunks[chunk.m_id.y + 1])
                                            SetBlock(*ch, {x, 0, z}, plant);
                                }
                                
                                blockState = biome->GetTopBlock(random);
                            }
                        }
                        else
                            blockState = biome->GetUnderWaterBlock(random);
                    }
                    else if(y < height) {
                        if (y >= height - 3)
                            blockState = BlockState(Block::DIRT);
                        else
                            blockState = BlockState(Block::STONE);
                    }

                    SetBlock(chunk, {x, dy, z}, blockState);
                }
            }

        // for (auto &tree : trees) {
        //     int x = tree.x;
        //     int z = tree.z;
        //
        //     getBiome(x, z).makeTree(m_random, *m_pChunk, x, tree.y, z);
        // }
    }

    void ChunkGenerator::SetBlock(Chunk& chunk, int3 pos, const BlockState& blockState) {
        chunk.m_blockStates[Chunk::ToIndex(pos)] = blockState;
    }

    void ChunkGenerator::GenerateBiomeMap(ChunkColumn& chunkColumn) {
        int2 columnID = chunkColumn.m_id;
        
        for (int z = 0; z < Config::CHUNK_SIZE.z + 1; z++)
            for (int x = 0; x < Config::CHUNK_SIZE.x + 1; x++) {
                i32 biomeValue = (i32)s_biomeNoise.GetHeight(x, z, columnID.x + 7, columnID.y + 7);
                const Biome* biome = nullptr;
                
                if (biomeValue > 160)
                    biome = &Biome::OCEAN;
                else if (biomeValue > 150)
                    biome = &Biome::PLAINS;
                else if (biomeValue > 130)
                    biome = &Biome::FOREST;
                else if (biomeValue > 120)
                    biome = &Biome::TAIGA_FOREST;
                else if (biomeValue > 110)
                    biome = &Biome::FOREST;
                else if (biomeValue > 100)
                    biome = &Biome::PLAINS;
                else
                    biome = &Biome::DESERT;

                chunkColumn.m_biomeMap[x + z * Config::CHUNK_SIZE.x] = biome;
            }
    }

    void ChunkGenerator::GenerateHeightMap(ChunkColumn& chunkColumn) {        
        constexpr static auto HALF_CHUNK = Config::CHUNK_SIZE / 2;
        constexpr static auto CHUNK = Config::CHUNK_SIZE;

        i32 maxHeights[5] = {
            GenerateHeightMapIn(chunkColumn, 0,             0,            HALF_CHUNK.x, HALF_CHUNK.z),
            GenerateHeightMapIn(chunkColumn, HALF_CHUNK.x,  0,            CHUNK.x,      HALF_CHUNK.z),
            GenerateHeightMapIn(chunkColumn, 0,             HALF_CHUNK.z, HALF_CHUNK.x, CHUNK.z),
            GenerateHeightMapIn(chunkColumn, HALF_CHUNK.x,  HALF_CHUNK.z, CHUNK.x,      CHUNK.z),
            Config::SEA_LEVEL,
        };

        chunkColumn.m_maxHeight = std::max(std::initializer_list(std::begin(maxHeights), std::end(maxHeights)));
    }

    i32 ChunkGenerator::GenerateHeightMapIn(ChunkColumn& chunkColumn, i32 xMin, i32 zMin, i32 xMax, i32 zMax) {
        i32 maxHeight = 0;
        
        auto getHeightAt = [&](int2 pos) {
            const Biome* biome = chunkColumn.m_biomeMap[pos.x + pos.y * Config::CHUNK_SIZE.x];
            return biome->GetHeight(pos, chunkColumn.m_id);
        };

        f32 bottomLeft = static_cast<f32>(getHeightAt({xMin, zMin}));
        f32 bottomRight = static_cast<f32>(getHeightAt({xMax, zMin}));
        f32 topLeft = static_cast<f32>(getHeightAt({xMin, zMax}));
        f32 topRight = static_cast<f32>(getHeightAt({xMax, zMax}));

        for (int x = xMin; x < xMax; ++x)
            for (int z = zMin; z < zMax; ++z) {
                if (x == Config::CHUNK_SIZE.x)
                    continue;
                if (z == Config::CHUNK_SIZE.z)
                    continue;

                i32 height = (i32)SmoothInterpolation(
                    bottomLeft, topLeft, bottomRight, topRight,
                    static_cast<f32>(xMin), static_cast<f32>(xMax),
                    static_cast<f32>(zMin), static_cast<f32>(zMax),
                    static_cast<f32>(x),    static_cast<f32>(z)
                );

                if(height > maxHeight)
                    maxHeight = height;

                chunkColumn.m_heightMap[x + z * Config::CHUNK_SIZE.x] = height;
            }
        return maxHeight;
    }

    float Clamp(float x, float lowerLimit, float upperLimit);

    float Smoothstep(float edge0, float edge1, float x)
    {
        // Scale, bias and saturate x to 0..1 range
        x = x * x * (3 - 2 * x);
        // Evaluate polynomial
        return (edge0 * x) + (edge1 * (1 - x));
    }

    float Clamp(float x, float lowerLimit, float upperLimit)
    {
        if (x < lowerLimit)
            x = lowerLimit;
        if (x > upperLimit)
            x = upperLimit;
        return x;
    }

    float SmoothInterpolation(float bottomLeft, float topLeft, float bottomRight, float topRight,
                              float xMin, float xMax, float zMin, float zMax,
                              float x, float z)
    {
        float width = xMax - xMin, height = zMax - zMin;
        float xValue = 1 - (x - xMin) / width;
        float zValue = 1 - (z - zMin) / height;

        float a = Smoothstep(bottomLeft, bottomRight, xValue);
        float b = Smoothstep(topLeft, topRight, xValue);
        return Smoothstep(a, b, zValue);
    }
}
