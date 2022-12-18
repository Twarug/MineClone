#include "mcpch.h"
#include "Block.h"

#include "MineClone/Config.h"
#include "MineClone/Game/Utils/Facing.h"

namespace mc
{
    const Block& Block::AIR = REGISTRY.Register(new Block("air", float3{}, true));
    const Block& Block::STONE = REGISTRY.Register(new Block("stone", float3(.5f)));
    const Block& Block::DIRT = REGISTRY.Register(new Block("dirt", float3(0.5f, 0.3f,0)));

    Block::Block(const std::string& name, float3 color, bool transparent)
        : RegistryEntry(name), m_transparent(transparent), m_color(color) {}

    float4 Block::GetTextureUVs(const Facing& facing) const {
        u64 x = m_id % Config::ATLAS_SIZE.x;
        u64 y = Config::ATLAS_SIZE.y - m_id / Config::ATLAS_SIZE.x;
        
        return {
            (f32)x * Config::ATLAS_ELEMENT_SIZE.x,
            (f32)y * Config::ATLAS_ELEMENT_SIZE.y,
            (f32)(x + 1) * Config::ATLAS_ELEMENT_SIZE.x,
            (f32)(y + 1) * Config::ATLAS_ELEMENT_SIZE.y,
        };
    }
}
