#pragma once

namespace mc
{
    struct Identifier
    {
        std::string name;

        constexpr bool operator<(const Identifier& r) const {
            return name < r.name;
        }
    };
}
