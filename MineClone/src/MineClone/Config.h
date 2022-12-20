#pragma once

#include "Core.h"

namespace mc
{
    struct Vertex3D;
    struct VertexDescription;
}


namespace mc
{
    class Config
    {
    public:
        static constexpr ulong3 WORLD_SIZE = {10, 5, 10};
        static constexpr int3 CHUNK_SIZE = {16, 16, 16};

        static constexpr u64 RENDER_DISTANCE = 1;

        static constexpr ulong2 ATLAS_SIZE = {2, 2};
        static constexpr float2 ATLAS_ELEMENT_SIZE = {1.f / ATLAS_SIZE.x, 1.f / ATLAS_SIZE.y};

        
        static const std::array<u32, 6ull * 6ull> INDICES;
        static const std::array<std::array<Vertex3D, 4>, 6> VERTICES;
        

    private:
        static constexpr u64 RENDER_DISTANCE_ARRAY_DIM = RENDER_DISTANCE * 2 + 1;
        static constexpr std::array<int3, RENDER_DISTANCE_ARRAY_DIM * RENDER_DISTANCE_ARRAY_DIM * RENDER_DISTANCE_ARRAY_DIM> GeneratePattern();

    public:
        static const std::array<int3, RENDER_DISTANCE_ARRAY_DIM * RENDER_DISTANCE_ARRAY_DIM * RENDER_DISTANCE_ARRAY_DIM> CHUNK_RENDER_PATTERN;
    };

    
    struct Vertex2D
    {
        float2 pos;
        float3 color;

        static VertexDescription GetDescription();
    };

    struct Vertex3D
    {
        float3 pos;
        float3 normal;
        float3 color;
        float2 uv;

        static VertexDescription GetDescription();
    };
}
