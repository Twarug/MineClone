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
        static constexpr ulong3 WORLD_SIZE = {3, 2, 3};
        static constexpr int3 CHUNK_SIZE = {16, 16, 16};

        static constexpr int RENDER_DISTANCE = 5;

        static const std::array<std::array<Vertex3D, 4>, 6> VERTICES;

        static const std::array<u32, 6ull * 6ull> INDICES;
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
