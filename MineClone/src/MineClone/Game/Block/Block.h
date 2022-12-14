#pragma once

#include "MineClone/Game/Registry/Registry.h"
#include "MineClone/Game/Registry/RegistryEntry.h"

namespace mc
{
    class Block : public RegistryEntry<Block>
    {
    public:
        // ReSharper disable once CppInconsistentNaming
        inline static Registry<Block> REGISTRY{};

        static const Block& AIR;
        static const Block& STONE;

    protected:
        explicit constexpr Block(const std::string& name, float3 color, bool transparent = false);

    public:
        bool IsTransparent() const { return m_transparent; }
        float3 GetColor() const { return m_color; }

    private:
        bool m_transparent;
        float3 m_color;
    };
}
