#include "mcpch.h"
#include "Block.h"

namespace mc
{
    const Block& Block::AIR = REGISTRY.Register(new Block("air", float3{}, true));
    const Block& Block::STONE = REGISTRY.Register(new Block("stone", float3(.5f)));
    const Block& Block::DIRT = REGISTRY.Register(new Block("dirt", float3(0.5f, 0.3f,0)));

    constexpr Block::Block(const std::string& name, float3 color, bool transparent)
        : RegistryEntry(name), m_transparent(transparent), m_color(color) {}
}
