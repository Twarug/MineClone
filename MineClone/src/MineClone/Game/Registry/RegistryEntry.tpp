#pragma once
#include "RegistryEntry.h"


namespace mc
{
    template <typename T>
    RegistryEntry<T>::RegistryEntry(std::string name)
        : m_identifier(std::move(name)) { }

    template <typename T>
    constexpr Identifier RegistryEntry<T>::GetId() const {
        return m_identifier;
    }

    template <typename T>
    constexpr const std::string& RegistryEntry<T>::GetName() const {
        return m_identifier.name;
    }
}
