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
        static const Block& GRASS_BLOCK;
        static const Block& OAK_LOG;

    protected:
        Block(const std::string& name, u64 textureIndex, bool transparent = false);
        Block(const std::string& name, std::array<u64, 6> textureIndexes, bool transparent = false);

    public:
        virtual void Tick(World& world, int3 pos, BlockState& blockState) const {}

    public:
        float4 GetTextureUVs(const Facing& facing) const;
        
    public:
        bool IsTransparent() const { return m_transparent; }
        
    private:
        std::array<u64, 6> m_textureIndexes;
        bool m_transparent;
    };
}
