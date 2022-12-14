#include "mcpch.h"
#include "Block.h"

namespace mc
{
    const Block& Block::AIR = REGISTRY.Register(new Block("air", float3{}, true));
    const Block& Block::STONE = REGISTRY.Register(new Block("stone", float3(.5f)));

    constexpr Block::Block(const std::string& name, float3 color, bool transparent)
        : RegistryEntry(name), m_transparent(transparent), m_color(color) {}
}
