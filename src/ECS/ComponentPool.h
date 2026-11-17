#pragma once

#include "Entity.h"
#include <vector>
#include <unordered_map>
#include <cassert>
#include <cstddef>

// Type-erased base for storing heterogeneous pools in a single container
class IComponentPool {
public:
    virtual ~IComponentPool() = default;
    virtual void Remove(EntityID entity) = 0;
    virtual bool Has(EntityID entity) const = 0;
    virtual size_t Size() const = 0;
};

template<typename T>
class ComponentPool : public IComponentPool {
public:
    // Add a component to an entity; returns a reference to the stored component
    template<typename... Args>
    T& Add(EntityID entity, Args&&... args) {
        assert(!Has(entity) && "Entity already has this component");

        size_t newIndex = m_DenseComponents.size();
        m_DenseComponents.emplace_back(std::forward<Args>(args)...);
        m_DenseEntities.push_back(entity);
        m_EntityToIndex[entity] = newIndex;

        return m_DenseComponents.back();
    }

    // Remove a component from an entity (swap-and-pop)
    void Remove(EntityID entity) override {
        assert(Has(entity) && "Entity does not have this component");

        auto it = m_EntityToIndex.find(entity);
        size_t removedIndex = it->second;
        size_t lastIndex    = m_DenseComponents.size() - 1;

        if (removedIndex != lastIndex) {
            // Swap with the last element
            m_DenseComponents[removedIndex] = std::move(m_DenseComponents[lastIndex]);
            m_DenseEntities[removedIndex]   = m_DenseEntities[lastIndex];

            // Update the mapping for the swapped entity
            EntityID swappedEntity = m_DenseEntities[removedIndex];
            m_EntityToIndex[swappedEntity] = removedIndex;
        }

        m_DenseComponents.pop_back();
        m_DenseEntities.pop_back();
        m_EntityToIndex.erase(it);
    }

    // Check if an entity has this component
    bool Has(EntityID entity) const override {
        return m_EntityToIndex.find(entity) != m_EntityToIndex.end();
    }

    // Get a reference to the entity's component
    T& Get(EntityID entity) {
        assert(Has(entity) && "Entity does not have this component");
        return m_DenseComponents[m_EntityToIndex[entity]];
    }

    const T& Get(EntityID entity) const {
        assert(Has(entity) && "Entity does not have this component");
        return m_DenseComponents[m_EntityToIndex.at(entity)];
    }

    // Size and iteration
    size_t Size() const override { return m_DenseComponents.size(); }

    // Direct access to dense arrays for system iteration
    const std::vector<EntityID>& GetEntities() const { return m_DenseEntities; }
    std::vector<T>&              GetComponents()       { return m_DenseComponents; }
    const std::vector<T>&        GetComponents() const { return m_DenseComponents; }

private:
    std::vector<T>        m_DenseComponents;   // contiguous component data
    std::vector<EntityID> m_DenseEntities;     // parallel array: entity ID at each index
    std::unordered_map<EntityID, size_t> m_EntityToIndex;  // sparse → dense mapping
};
