#include "mcpch.h"
#include "NoiseGenerator.h"

#include "MineClone/Config.h"

namespace mc
{
    NoiseGenerator::NoiseGenerator(i32 seed)
        :  m_noiseParameters{}, m_seed(seed)
    {
        m_noiseParameters.octaves = 7;
        m_noiseParameters.amplitude = 70;
        m_noiseParameters.smoothness = 235;
        m_noiseParameters.heightOffset = -5;
        m_noiseParameters.roughness = 0.53f;
    }

    void NoiseGenerator::SetParameters(const NoiseParameters &params) noexcept
    {
        m_noiseParameters = params;
    }

    f32 NoiseGenerator::GetNoise(i32 n) const noexcept
    {
        n += m_seed;
        n = (n << 13) ^ n;
        auto newN = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;

        return 1.0f - ((f32)newN / 1073741824.0f);
    }

    f32 NoiseGenerator::GetNoise(f32 x, f32 z) const noexcept
    {
        return GetNoise((i32)(x + z * 57.0f));
    }

    f32 NoiseGenerator::Lerp(f32 a, f32 b, f32 z) const noexcept
    {
        f32 mu2 = (1 - std::cos(z * 3.14f)) / 2;
        return (a * (1 - mu2) + b * mu2);
    }

    f32 NoiseGenerator::Noise(f32 x, f32 z) const noexcept
    {
        f32 floorX = (f32)(i32)x; // This is kinda a cheap way to floor a float integer.
        f32 floorZ = (f32)(i32)z;

        f32 s = GetNoise(floorX, floorZ);
        f32 t = GetNoise(floorX + 1, floorZ);
        f32 u = GetNoise(floorX, floorZ + 1); // Get the surrounding values to calculate the transition.
        f32 v = GetNoise(floorX + 1, floorZ + 1);

        auto rec1 = Lerp(s, t, x - floorX); // Interpolate between the values.
        auto rec2 = Lerp(u, v, x - floorX); // Here we use x-floorX, to get 1st dimension. Don't mind

        // the x-floorX thingie, it's part of the cosine formula.
        auto rec3 = Lerp(rec1, rec2, z - floorZ); // Here we use y-floorZ, to get the 2nd dimension.
        return rec3;
    }

    f32 NoiseGenerator::GetHeight(i32 x, i32 z, i32 chunkX, i32 chunkZ) const noexcept
    {
        auto newX = (x + (chunkX * Config::CHUNK_SIZE.x));
        auto newZ = (z + (chunkZ * Config::CHUNK_SIZE.z));

        // if (newX < 0 || newZ < 0)
        //     return 0;

        f32 totalValue = 0.f;

        for (auto a = 0; a < m_noiseParameters.octaves - 1; a++) // This loops through the octaves.
        {
            f32 frequency = (f32)std::pow(2.f, a); // This increases the frequency with every loop of the octave.
            f32 amplitude = (f32)std::pow(m_noiseParameters.roughness, a); // This decreases the amplitude with every loop of the octave.
            totalValue += Noise(((f32)newX) * frequency / (f32)m_noiseParameters.smoothness, ((f32)newZ) * frequency / (f32)m_noiseParameters.smoothness) * amplitude;
        }

        f32 val = (((totalValue / 2.1f) + 1.2f) * (f32)m_noiseParameters.amplitude) + (f32)m_noiseParameters.heightOffset;

        return val > 0 ? val : 1;
    }
}
