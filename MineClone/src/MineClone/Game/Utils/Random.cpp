#include "mcpch.h"
#include "Random.h"

namespace mc
{
    RandomSingleton &RandomSingleton::Get()
    {
        static RandomSingleton r;
        return r;
    }

    RandomSingleton::RandomSingleton()
    {
        m_randomEngine.seed(static_cast<unsigned>(std::time(nullptr)));
        for (int i = 0; i < 5; i++)
            Range(i, i * 5);
    }
}