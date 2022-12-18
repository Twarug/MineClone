#pragma once


namespace mc
{    
    class Facing
    {
    public:
        static const Facing UP;
        static const Facing DOWN;
        static const Facing NORTH;
        static const Facing SOUTH;
        static const Facing EAST;
        static const Facing WEST;
        
        static const std::array<Facing, 6> FACINGS;

    private:
        constexpr Facing(u32 index, u32 opposite, i32 horizontalIndex, std::string name, int3 directionVec)
            : index(index), opposite(opposite), horizontalIndex(horizontalIndex), name(std::move(name)), directionVec(directionVec) {}

        constexpr Facing(const Facing&) noexcept = default;
        constexpr Facing(Facing&&) noexcept = default;
        
    public:
        u32 index;
        u32 opposite;
        i32 horizontalIndex;
        std::string name;
        int3 directionVec;
    };
}
