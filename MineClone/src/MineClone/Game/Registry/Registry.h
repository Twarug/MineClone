#pragma once
#include "Identifier.h"
#include "MineClone/Game/Block/Block.h"

namespace mc
{
    template <typename T>
    class Registry
    {
    public:
        constexpr Registry() = default;
        constexpr ~Registry() = default;

        Registry(const Registry& other) = delete;
        Registry(Registry&& other) noexcept = delete;
        Registry& operator=(const Registry& other) = delete;
        Registry& operator=(Registry&& other) noexcept = delete;

    public:
        constexpr const T& Register(T* entry);

        constexpr const T& GetByID(Identifier id);

    private:
        std::map<Identifier, T*> m_registry{};
    };
}

#include "Registry.tpp"
