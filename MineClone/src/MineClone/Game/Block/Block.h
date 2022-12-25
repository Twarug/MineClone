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
        static const Block& OAK_LOG;

    protected:
        Block(const std::string& name, int textureIndex, bool transparent = false);

    public:
        virtual void Tick(World& world, int3 pos, BlockState& blockState) const {}

    public:
        float4 GetTextureUVs(const Facing& facing) const;
        
    public:
        bool IsTransparent() const { return m_transparent; }
        
    private:
        u64 m_textureIndex;
        bool m_transparent;
    };
}
