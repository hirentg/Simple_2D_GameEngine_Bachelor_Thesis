#include "SpatialHashGrid.h"
#include <algorithm>

SpatialHashGrid::SpatialHashGrid(float cellSize)
    : m_CellSize(cellSize), m_InvCellSize(1.0f / cellSize)
{
}

void SpatialHashGrid::Clear() {
    // Clear each cell's entity list without deallocating the map structure.
    // Since the grid is rebuilt every physics frame (60 Hz), reusing the
    // existing buckets avoids repeated allocation/deallocation overhead.
    m_Cells.clear();
}

CellKey SpatialHashGrid::WorldToCell(const glm::vec2& position) const {
    return {
        static_cast<int32_t>(std::floor(position.x * m_InvCellSize)),
        static_cast<int32_t>(std::floor(position.y * m_InvCellSize))
    };
}

void SpatialHashGrid::Insert(EntityID entity,
                              const glm::vec2& min, const glm::vec2& max)
{
    CellKey minCell = WorldToCell(min);
    CellKey maxCell = WorldToCell(max);

    // Insert entity into every cell its AABB overlaps
    for (int32_t y = minCell.y; y <= maxCell.y; y++) {
        for (int32_t x = minCell.x; x <= maxCell.x; x++) {
            m_Cells[{x, y}].push_back(entity);
        }
    }
}

std::vector<EntityPair> SpatialHashGrid::GetCandidatePairs() const {
    // Use a set to avoid duplicate pairs
    std::unordered_set<EntityPair, EntityPairHash> uniquePairs;

    for (const auto& [cellKey, entities] : m_Cells) {
        for (size_t i = 0; i < entities.size(); i++) {
            for (size_t j = i + 1; j < entities.size(); j++) {
                EntityID a = entities[i];
                EntityID b = entities[j];

                // Canonical ordering: smaller ID first
                if (a > b) std::swap(a, b);
                uniquePairs.insert({a, b});
            }
        }
    }

    std::vector<EntityPair> result(uniquePairs.begin(), uniquePairs.end());
 std::sort(result.begin(),result.end());return result;
}
