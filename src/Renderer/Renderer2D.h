#pragma once

#include <memory>
#include <array>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Texture2D.h"
#include "OrthographicCamera.h"

struct QuadVertex {
    glm::vec3 Position;
    glm::vec4 Color;
    glm::vec2 TexCoords;
    float     TexIndex;
};

struct Renderer2DStats {
    uint32_t DrawCalls = 0;
    uint32_t QuadCount = 0;
};

class Renderer2D {
public:
    static void Init();
    static void Shutdown();

    static void BeginBatch(const OrthographicCamera& camera);
    static void EndBatch();
    static void Flush();

    // Draw a colored quad (no texture)
    static void DrawQuad(const glm::vec2& position, const glm::vec2& size,
                         const glm::vec4& color);

    // Draw a textured quad
    static void DrawQuad(const glm::vec2& position, const glm::vec2& size,
                         const std::shared_ptr<Texture2D>& texture,
                         const glm::vec4& tintColor = glm::vec4(1.0f));

    // Draw a textured quad with rotation
    static void DrawQuad(const glm::vec2& position, const glm::vec2& size,
                         float rotation,
                         const std::shared_ptr<Texture2D>& texture,
                         const glm::vec4& tintColor = glm::vec4(1.0f),
                         const glm::vec4& uvRect = {0.0f, 0.0f, 1.0f, 1.0f});

    static Renderer2DStats GetStats();
    static void ResetStats();

private:
    static void StartNewBatch();
    static float GetTextureIndex(const std::shared_ptr<Texture2D>& texture);
};
