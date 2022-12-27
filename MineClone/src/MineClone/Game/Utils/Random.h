#pragma once

#include <random>

namespace mc
{
    class RandomSingleton
    {
    public:
        static RandomSingleton& Get();

        template <std::integral T> T Range(T low, T high)
        {
            std::uniform_int_distribution<T> dist(low, high);
            return dist(m_randomEngine);
        }

    private:
        RandomSingleton();

        std::mt19937 m_randomEngine;
    };

    template <typename REngine = std::mt19937>
    class Random
    {
    public:
        Random(i32 n = (i32)std::time(nullptr))
        {
            m_randomEngine.seed(n);
            for (int i = 0; i < 5; i++)
                Range(i, i * 5);
        }

        template <std::integral T> T Range(T low, T high)
        {
            std::uniform_int_distribution<T> dist(low, high);
            return dist(m_randomEngine);
        }

        void SetSeed(int seed)
        {
            m_randomEngine.seed(seed);
        }

    private:
        REngine m_randomEngine;
    };
}