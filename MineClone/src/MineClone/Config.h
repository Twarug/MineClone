#pragma once

#include "Core.h"

namespace mc
{
    class Config
    {
    public:
        static constexpr ulong3 WORLD_SIZE = {5, 16, 5};
        static constexpr uint3 CHUNK_SIZE = {16, 16, 16};

        static constexpr int RENDER_DISTANCE = 5;
    };
}
