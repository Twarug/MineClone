#pragma once

namespace mc
{
    struct NoiseParameters {
        i32 octaves;
        i32 amplitude;
        i32 smoothness;
        i32 heightOffset;

        f32 roughness;
    };

    class NoiseGenerator {
    public:
        NoiseGenerator(i32 seed);

        f32 GetHeight(i32 x, i32 z, i32 chunkX, i32 chunkZ) const noexcept;

        void SetParameters(const NoiseParameters &params) noexcept;

    private:
        f32 GetNoise(i32 n) const noexcept;
        f32 GetNoise(f32 x, f32 z) const noexcept;

        f32 Lerp(f32 a, f32 b, f32 z) const noexcept;

        f32 Noise(f32 x, f32 z) const noexcept;

        NoiseParameters m_noiseParameters;

        int m_seed;
    };
}
