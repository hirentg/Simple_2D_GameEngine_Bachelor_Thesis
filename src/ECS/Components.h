#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "../Renderer/Texture2D.h"

// ── Transform ──────────────────────────────────────────────────────────────────
// Explicit constructors are required because C++17 does not support parenthesized
// aggregate initialization. Without them, emplace_back(args...) in ComponentPool
// would fail to compile.
struct TransformComponent {
    glm::vec2 position = {0.0f, 0.0f};
    float     rotation = 0.0f;           // degrees
    glm::vec2 scale    = {1.0f, 1.0f};

    TransformComponent() = default;
    TransformComponent(const glm::vec2& pos, float rot, const glm::vec2& sc)
        : position(pos), rotation(rot), scale(sc) {}
};

// ── Sprite ─────────────────────────────────────────────────────────────────────
struct SpriteComponent {
    glm::vec4                    color   = {1.0f, 1.0f, 1.0f, 1.0f};  // RGBA tint
    std::shared_ptr<Texture2D>   texture = nullptr;
    glm::vec4                    uvRect  = {0.0f, 0.0f, 1.0f, 1.0f};  // atlas sub-region

    SpriteComponent() = default;
    SpriteComponent(const glm::vec4& col, std::shared_ptr<Texture2D> tex,
                    const glm::vec4& uv)
        : color(col), texture(std::move(tex)), uvRect(uv) {}
};

// ── Rigidbody ──────────────────────────────────────────────────────────────────
struct RigidbodyComponent {
    glm::vec2 velocity     = {0.0f, 0.0f};
    glm::vec2 acceleration = {0.0f, 0.0f};
    float     mass         = 1.0f;
    float     restitution  = 0.5f;   // bounciness [0, 1]
    bool      isStatic     = false;  // static bodies are immovable

    RigidbodyComponent() = default;
    RigidbodyComponent(const glm::vec2& vel, const glm::vec2& accel,
                       float m, float r, bool s)
        : velocity(vel), acceleration(accel), mass(m), restitution(r), isStatic(s) {}
};

// ── Collider (AABB) ────────────────────────────────────────────────────────────
struct ColliderComponent {
    glm::vec2 halfExtents = {0.5f, 0.5f};  // half-width, half-height
    glm::vec2 offset      = {0.0f, 0.0f};  // offset from transform position
    uint16_t  layerMask   = 0xFFFF;        // collision layer bitmask

    ColliderComponent() = default;
    ColliderComponent(const glm::vec2& he, const glm::vec2& off, uint16_t lm)
        : halfExtents(he), offset(off), layerMask(lm) {}
};


// ── Audio Source ────────────────────────────────────────────────────────────────
struct AudioSourceComponent {
    std::string clipPath;
    float       volume      = 1.0f;
    bool        loop        = false;
    bool        playOnStart = false;
    bool        isPlaying   = false;
    uint32_t    soundHandle = 0;     // handle returned by AudioEngine::PlaySound

    AudioSourceComponent() = default;
    AudioSourceComponent(const std::string& path, float vol, bool lp, bool autoPlay)
        : clipPath(path), volume(vol), loop(lp), playOnStart(autoPlay) {}
};
