#pragma once

namespace mc
{
    struct Identifier
    {
        std::string name;
        
        std::strong_ordering operator<=>(const Identifier& identifier) const = default;
    };
}
