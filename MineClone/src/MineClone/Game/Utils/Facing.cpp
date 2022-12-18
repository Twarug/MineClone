#include "mcpch.h"
#include "Facing.h"

namespace mc
{
    const Facing Facing::UP    = {0, 1, -1, "Down",  { 0,  1,  0}};
    const Facing Facing::DOWN  = {1, 0, -1, "Up",    { 0, -1,  0}};
    const Facing Facing::NORTH = {2, 3,  2, "North", { 0,  0,  1}};
    const Facing Facing::SOUTH = {3, 2,  0, "South", { 0,  0, -1}};
    const Facing Facing::EAST  = {4, 5,  1, "West",  { 1,  0,  0}};
    const Facing Facing::WEST  = {5, 4,  3, "East",  {-1,  0,  0}};

    const std::array<Facing, 6> Facing::FACINGS = {
        UP, DOWN, NORTH, SOUTH, EAST, WEST,
    };
}
