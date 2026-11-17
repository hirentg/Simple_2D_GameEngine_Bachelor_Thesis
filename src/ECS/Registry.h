#pragma once

#include "Entity.h"
#include "ComponentPool.h"
#include <queue>
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <functional>

class Registry {
public:
    // ── Entity management ──────────────────────────────────────────────────
    EntityID CreateEntity() {
        EntityID id;
        if (!m_RecycledIDs.empty()) {
            id = m_RecycledIDs.front();
            m_RecycledIDs.pop();
        } else {
            id = m_NextEntityID++;
        }
        m_AliveEntities.insert(id);
        return id;
    }

    void DestroyEntity(EntityID entity) {
        // Deferred: add to pending destruction queue
        m_PendingDestruction.push_back(entity);
    }

    // Called at the end of each frame to actually destroy pending entities
    void FlushDestroyedEntities() {
        for (EntityID entity : m_PendingDestruction) {
 if (!IsAlive(entity)) continue;
            // Remove all components belonging to this entity
            for (auto& [typeIdx, pool] : m_ComponentPools) {
                if (pool->Has(entity)) {
                    pool->Remove(entity);
                }
            }
            // Remove from alive set — O(1) average
            m_AliveEntities.erase(entity);
            m_RecycledIDs.push(entity);
        }
        m_PendingDestruction.clear();
    }

    bool IsAlive(EntityID entity) const {
        return m_AliveEntities.count(entity) > 0;  // O(1) average
    }

    size_t EntityCount() const { return m_AliveEntities.size(); }

    // ── Component management ───────────────────────────────────────────────
    template<typename T, typename... Args>
    T& AddComponent(EntityID entity, Args&&... args) {
        return GetOrCreatePool<T>()->Add(entity, std::forward<Args>(args)...);
    }

    template<typename T>
    void RemoveComponent(EntityID entity) {
        GetOrCreatePool<T>()->Remove(entity);
    }

    template<typename T>
    T& GetComponent(EntityID entity) {
        return GetOrCreatePool<T>()->Get(entity);
    }

    template<typename T>
    const T& GetComponent(EntityID entity) const {
        return GetPool<T>()->Get(entity);
    }

    template<typename T>
    bool HasComponent(EntityID entity) const {
        auto* pool = GetPool<T>();
        return pool && pool->Has(entity);
    }

    // ── View: iterate over entities with all of the specified components ───
    // Usage: for (EntityID e : registry.View<Transform, Sprite>()) { ... }
    template<typename... Components>
    class View {
    public:
        View(Registry& registry) : m_Registry(registry) {
            // Cache the smallest pool's entity list at construction time
            // instead of recomputing it on every begin()/end() call.
            m_CachedEntities = &m_EmptyEntities;
            size_t minSize = SIZE_MAX;
            auto findSmallest = [&](auto* pool) {
                if (pool && pool->Size() < minSize) {
                    minSize = pool->Size();
                    m_CachedEntities = &pool->GetEntities();
                }
            };
            (findSmallest(m_Registry.GetOrCreatePool<Components>()), ...);
        }

        class Iterator {
        public:
            Iterator(Registry& reg, const std::vector<EntityID>& entities, size_t index)
                : m_Registry(reg), m_Entities(entities), m_Index(index)
            {
                AdvanceToValid();
            }

            EntityID operator*() const { return m_Entities[m_Index]; }

            Iterator& operator++() {
                m_Index++;
                AdvanceToValid();
                return *this;
            }

            bool operator!=(const Iterator& other) const {
                return m_Index != other.m_Index;
            }

        private:
            void AdvanceToValid() {
                while (m_Index < m_Entities.size()) {
                    EntityID e = m_Entities[m_Index];
                    if ((m_Registry.HasComponent<Components>(e) && ...)) {
                        break;  // found a valid entity
                    }
                    m_Index++;
                }
            }

            Registry& m_Registry;
            const std::vector<EntityID>& m_Entities;
            size_t m_Index;
        };

        Iterator begin() {
            if (m_CachedEntities->empty())
                return end();
            return Iterator(m_Registry, *m_CachedEntities, 0);
        }

        Iterator end() {
            return Iterator(m_Registry, *m_CachedEntities,
                            m_CachedEntities->size());
        }

    private:
        Registry& m_Registry;
        std::vector<EntityID> m_EmptyEntities;
        const std::vector<EntityID>* m_CachedEntities;
    };

    template<typename... Components>
    View<Components...> GetView() {
        return View<Components...>(*this);
    }

    // ── Pool access (internal) ─────────────────────────────────────────────
    template<typename T>
    ComponentPool<T>* GetOrCreatePool() {
        std::type_index typeIdx(typeid(T));
        auto it = m_ComponentPools.find(typeIdx);
        if (it == m_ComponentPools.end()) {
            auto pool = std::make_unique<ComponentPool<T>>();
            auto* rawPtr = pool.get();
            m_ComponentPools[typeIdx] = std::move(pool);
            return rawPtr;
        }
        return static_cast<ComponentPool<T>*>(it->second.get());
    }

    template<typename T>
    IComponentPool* GetPoolBase() {
        std::type_index typeIdx(typeid(T));
        auto it = m_ComponentPools.find(typeIdx);
        return (it != m_ComponentPools.end()) ? it->second.get() : nullptr;
    }

private:
    template<typename T>
    const ComponentPool<T>* GetPool() const {
        std::type_index typeIdx(typeid(T));
        auto it = m_ComponentPools.find(typeIdx);
        if (it == m_ComponentPools.end()) return nullptr;
        return static_cast<const ComponentPool<T>*>(it->second.get());
    }

    EntityID m_NextEntityID = 0;
    std::queue<EntityID> m_RecycledIDs;
    std::unordered_set<EntityID> m_AliveEntities;    // O(1) lookup
    std::vector<EntityID> m_PendingDestruction;

    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> m_ComponentPools;
};
