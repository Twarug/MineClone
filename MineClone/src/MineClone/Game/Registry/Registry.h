#pragma once
#include "Identifier.h"

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

    public:
        constexpr auto begin() { return m_registry.begin(); }
        constexpr auto end()  { return m_registry.end(); }

        constexpr auto begin() const { return m_registry.begin(); }
        constexpr auto end() const { return m_registry.end(); }
        
    private:
        std::map<Identifier, T*> m_registry{};
    };
}

#include "Registry.tpp"
