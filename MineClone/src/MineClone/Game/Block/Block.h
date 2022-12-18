#pragma once

#include "MineClone/Game/Registry/Registry.h"
#include "MineClone/Game/Registry/RegistryEntry.h"
#include "MineClone/Game/Utils/Facing.h"

namespace mc
{
    class World;
    class BlockState;

    class Block : public RegistryEntry<Block>
    {
    public:
        // ReSharper disable once CppInconsistentNaming
        inline static Registry<Block> REGISTRY{};

        static const Block& AIR;
        static const Block& STONE;
        static const Block& DIRT;

    protected:
        Block(const std::string& name, float3 color, bool transparent = false);

    public:
        virtual void Tick(World& world, int3 pos, BlockState& blockState) const {}

    public:
        float4 GetTextureUVs(const Facing& facing) const;
        
    public:
        bool IsTransparent() const { return m_transparent; }
        float3 GetColor() const { return m_color; }

    private:
        bool m_transparent;
        float3 m_color;

    private:
        inline static u64 s_id = 0;
        u64 m_id = s_id++;
    };
}
