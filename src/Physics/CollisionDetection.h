#pragma once

#include <glm/glm.hpp>

struct CollisionResult {
    bool      collided = false;
    glm::vec2 normal   = {0.0f, 0.0f};  // collision normal (A → B)
    float     depth    = 0.0f;           // penetration depth along the normal
};

// Test two AABBs for overlap. Returns collision normal and penetration depth
// using the Minimum Translation Vector (MTV) approach.
inline CollisionResult AABBvsAABB(const glm::vec2& aMin, const glm::vec2& aMax,
                                   const glm::vec2& bMin, const glm::vec2& bMax)
{
    CollisionResult result;

    // Check for separation on each axis
    float overlapX = glm::min(aMax.x, bMax.x) - glm::max(aMin.x, bMin.x);
    float overlapY = glm::min(aMax.y, bMax.y) - glm::max(aMin.y, bMin.y);

    if (overlapX <= 0.0f || overlapY <= 0.0f) {
        return result;  // no collision
    }

    result.collided = true;

    // MTV: use the axis with the smallest overlap
    if (overlapX < overlapY) {
        result.depth = overlapX;
        // Normal points from A center to B center on the X axis
        float aCenterX = (aMin.x + aMax.x) * 0.5f;
        float bCenterX = (bMin.x + bMax.x) * 0.5f;
        result.normal = (bCenterX > aCenterX) ? glm::vec2{1.0f, 0.0f}
                                               : glm::vec2{-1.0f, 0.0f};
    } else {
        result.depth = overlapY;
        float aCenterY = (aMin.y + aMax.y) * 0.5f;
        float bCenterY = (bMin.y + bMax.y) * 0.5f;
        result.normal = (bCenterY > aCenterY) ? glm::vec2{0.0f, 1.0f}
                                               : glm::vec2{0.0f, -1.0f};
    }

    return result;
}
