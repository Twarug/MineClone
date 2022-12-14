#pragma once
#include "Registry.h"

namespace mc
{
    template <typename T>
    constexpr const T& Registry<T>::Register(T* entry) {
        m_registry.emplace(entry->m_identifier, entry);

        return *entry;
    }

    template <typename T>
    constexpr const T& Registry<T>::GetByID(Identifier id) {
        return *m_registry.at(id);
    }
}
