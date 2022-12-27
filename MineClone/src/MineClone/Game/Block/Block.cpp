#include "mcpch.h"
#include "Block.h"

#include "MineClone/Config.h"
#include "MineClone/Game/Utils/Facing.h"

namespace mc
{
    const Block& Block::AIR = REGISTRY.Register(new Block("air", -1, true));
    const Block& Block::STONE = REGISTRY.Register(new Block("stone", 1));
    const Block& Block::GRASS_BLOCK = REGISTRY.Register(new Block("grass_block", {{0, 2, 3, 3, 3, 3}}));
    const Block& Block::DIRT = REGISTRY.Register(new Block("dirt", 2));
    const Block& Block::SAND = REGISTRY.Register(new Block("sand", 18));
    const Block& Block::OAK_LOG = REGISTRY.Register(new Block("oak_log", {{20, 20, 21, 21, 21, 21}}));

    Block::Block(const std::string& name, u64 textureIndex, bool transparent)
        : Block(name, std::array<u64, 6>{{textureIndex, textureIndex, textureIndex, textureIndex, textureIndex, textureIndex}}, transparent) {}

    Block::Block(const std::string& name, std::array<u64, 6> textureIndexes, bool transparent)
        : RegistryEntry(name), m_textureIndexes(textureIndexes), m_transparent(transparent) {}

    float4 Block::GetTextureUVs(const Facing& facing) const {
        u64 x = m_textureIndexes[facing.index] % Config::ATLAS_SIZE.x;
        u64 y = Config::ATLAS_SIZE.y - m_textureIndexes[facing.index] / Config::ATLAS_SIZE.x;
        
        return {
            (f32)x * Config::ATLAS_ELEMENT_SIZE.x,
            (f32)(y - 1) * Config::ATLAS_ELEMENT_SIZE.y,
            (f32)(x + 1) * Config::ATLAS_ELEMENT_SIZE.x,
            (f32)y * Config::ATLAS_ELEMENT_SIZE.y,
        };
    }
}
