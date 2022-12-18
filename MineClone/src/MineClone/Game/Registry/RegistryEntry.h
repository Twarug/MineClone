#pragma once

#include "Identifier.h"

namespace mc
{
    template <typename T>
    class Registry;


    template <typename T>
    class RegistryEntry
    {
    protected:
        explicit RegistryEntry(std::string name);
        virtual ~RegistryEntry() = default;

    public:
        RegistryEntry(const RegistryEntry& other) = delete;
        RegistryEntry(RegistryEntry&& other) noexcept = delete;
        RegistryEntry& operator=(const RegistryEntry& other) = delete;
        RegistryEntry& operator=(RegistryEntry&& other) noexcept = delete;

    public:
        constexpr Identifier GetId() const;
        constexpr const std::string& GetName() const;

    private:
        Identifier m_identifier;

        template <typename T1>
        friend class Registry;
    };
}

#include "RegistryEntry.tpp"
