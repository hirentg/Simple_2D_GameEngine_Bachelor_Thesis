#pragma once

#include "CollisionDetection.h"
#include "../ECS/Components.h"
#include <algorithm>

// Position correction bias: only correct a fraction of the penetration per frame
// to prevent jitter when multiple collisions occur simultaneously.
// Value from Erin Catto's GDC talks on Box2D.
static constexpr float CORRECTION_PERCENT = 0.8f;

// Minimum penetration threshold below which no positional correction is applied,
// to avoid micro-corrections that cause visible vibration.
static constexpr float CORRECTION_SLOP = 0.01f;

inline void ResolveCollision(TransformComponent& tfA, RigidbodyComponent& rbA,
                              ColliderComponent& colA,
                              TransformComponent& tfB, RigidbodyComponent& rbB,
                              ColliderComponent& colB,
                              const CollisionResult& collision)
{
    // ── Check layer mask compatibility ──────────────────────────────────────
    if ((colA.layerMask & colB.layerMask) == 0)
        return;  // these layers don't interact

    // ── Inverse mass (static bodies have inverse mass = 0) ─────────────────
    float invMassA = rbA.isStatic ? 0.0f : (1.0f / rbA.mass);
    float invMassB = rbB.isStatic ? 0.0f : (1.0f / rbB.mass);
    float invMassSum = invMassA + invMassB;

    if (invMassSum == 0.0f)
        return;  // both static — nothing to resolve

    // ── Stage 1: Positional correction ─────────────────────────────────────
    float correctionMagnitude = std::max(collision.depth - CORRECTION_SLOP, 0.0f)
                                * CORRECTION_PERCENT / invMassSum;

    glm::vec2 correction = correctionMagnitude * collision.normal;
    tfA.position -= invMassA * correction;
    tfB.position += invMassB * correction;

    // ── Stage 2: Velocity reflection (impulse resolution) ──────────────────
    glm::vec2 relativeVelocity = rbB.velocity - rbA.velocity;
    float normalVelocity = glm::dot(relativeVelocity, collision.normal);

    // If bodies are already separating, skip impulse
    if (normalVelocity > 0.0f)
        return;

    // Coefficient of restitution: use the minimum of the two bodies
    float e = std::min(rbA.restitution, rbB.restitution);

    // Impulse magnitude
    float j = -(1.0f + e) * normalVelocity / invMassSum;

    // Apply impulse to velocities
    glm::vec2 impulse = j * collision.normal;
    if (!rbA.isStatic) rbA.velocity -= invMassA * impulse;
    if (!rbB.isStatic) rbB.velocity += invMassB * impulse;
}
