#pragma once

#include "../ECS/Entity.h"
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <cstdint>
#include <utility>

struct CellKey {
    int32_t x;
    int32_t y;

    bool operator==(const CellKey& other) const {
        return x == other.x && y == other.y;
    }
};

// Custom hash using prime multiplication to reduce diagonal collisions
struct CellKeyHash {
    size_t operator()(const CellKey& key) const {
        // From GPU Gems — good spatial distribution
        return static_cast<size_t>(
            uint32_t(key.x) * 73856093u ^ uint32_t(key.y) * 19349663u
        );
    }
};

using EntityPair = std::pair<EntityID, EntityID>;

struct EntityPairHash {
    size_t operator()(const EntityPair& p) const {
        return std::hash<EntityID>()(p.first) ^ (std::hash<EntityID>()(p.second) << 16);
    }
};

class SpatialHashGrid {
public:
    SpatialHashGrid(float cellSize = 64.0f);

    // Clear the grid (called at the start of each physics frame)
    void Clear();

    // Insert an entity with the given AABB into the grid
    void Insert(EntityID entity, const glm::vec2& min, const glm::vec2& max);

    // Get all unique candidate collision pairs
    std::vector<EntityPair> GetCandidatePairs() const;

private:
    float m_CellSize;
    float m_InvCellSize;  // 1.0 / cellSize, cached to avoid division

    std::unordered_map<CellKey, std::vector<EntityID>, CellKeyHash> m_Cells;

    CellKey WorldToCell(const glm::vec2& position) const;
};
