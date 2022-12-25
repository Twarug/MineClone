#include "mcpch.h"
#include "Block.h"

#include "MineClone/Config.h"
#include "MineClone/Game/Utils/Facing.h"

namespace mc
{
    const Block& Block::AIR = REGISTRY.Register(new Block("air", -1, true));
    const Block& Block::STONE = REGISTRY.Register(new Block("stone", 1));
    const Block& Block::DIRT = REGISTRY.Register(new Block("dirt", 2));
    const Block& Block::OAK_LOG = REGISTRY.Register(new Block("oak_log", 21));

    Block::Block(const std::string& name, int textureIndex, bool transparent)
        : RegistryEntry(name), m_textureIndex(textureIndex), m_transparent(transparent) {}

    float4 Block::GetTextureUVs(const Facing& facing) const {
        u64 x = m_textureIndex % Config::ATLAS_SIZE.x;
        u64 y = Config::ATLAS_SIZE.y - m_textureIndex / Config::ATLAS_SIZE.x;
        
        return {
            (f32)x * Config::ATLAS_ELEMENT_SIZE.x,
            (f32)(y - 1) * Config::ATLAS_ELEMENT_SIZE.y,
            (f32)(x + 1) * Config::ATLAS_ELEMENT_SIZE.x,
            (f32)y * Config::ATLAS_ELEMENT_SIZE.y,
        };
    }
}
