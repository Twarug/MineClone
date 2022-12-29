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
        
        friend std::strong_ordering operator<=>(const RegistryEntry& entry, const RegistryEntry& otherEntry) {
            return &entry <=> &otherEntry;
        }

        bool operator==(const RegistryEntry& oth) const {
            return *this <=> oth == 0;
        }
    

    private:
        Identifier m_identifier;

        friend class Registry<T>;
    };
}

#include "RegistryEntry.tpp"
