#include "Renderer2D.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// ── Internal storage ───────────────────────────────────────────────────────────
static const uint32_t MaxQuads    = 10000;
static const uint32_t MaxVertices = MaxQuads * 4;
static const uint32_t MaxIndices  = MaxQuads * 6;
static const uint32_t MaxTextureSlots = 16;  // most GPUs support at least 32

struct Renderer2DData {
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    std::unique_ptr<Shader> QuadShader;
    std::shared_ptr<Texture2D> WhiteTexture;

    QuadVertex* VertexBufferBase = nullptr;
    QuadVertex* VertexBufferPtr  = nullptr;
    uint32_t    QuadCount = 0;

    std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> TextureSlots;
    uint32_t TextureSlotIndex = 1;  // slot 0 = white texture

    Renderer2DStats Stats;
};

static Renderer2DData s_Data;

// ── Init / Shutdown ────────────────────────────────────────────────────────────
void Renderer2D::Init() {
    // Create VAO
    glGenVertexArrays(1, &s_Data.VAO);
    glBindVertexArray(s_Data.VAO);

    // Create VBO (dynamic, written to every frame)
    glGenBuffers(1, &s_Data.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, s_Data.VBO);
    glBufferData(GL_ARRAY_BUFFER, MaxVertices * sizeof(QuadVertex),
                 nullptr, GL_DYNAMIC_DRAW);

    // Vertex attributes
    // Position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex),
                          (const void*)offsetof(QuadVertex, Position));
    // Color (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVertex),
                          (const void*)offsetof(QuadVertex, Color));
    // TexCoords (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex),
                          (const void*)offsetof(QuadVertex, TexCoords));
    // TexIndex (location 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(QuadVertex),
                          (const void*)offsetof(QuadVertex, TexIndex));

    // Create EBO (static — the index pattern never changes)
    auto* indices = new uint32_t[MaxIndices];
    uint32_t offset = 0;
    for (uint32_t i = 0; i < MaxIndices; i += 6) {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;
        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;
        offset += 4;
    }
    glGenBuffers(1, &s_Data.EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Data.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MaxIndices * sizeof(uint32_t),
                 indices, GL_STATIC_DRAW);
    delete[] indices;

    // CPU-side vertex buffer
    s_Data.VertexBufferBase = new QuadVertex[MaxVertices];

    // 1x1 white texture (used for untextured colored quads)
    s_Data.WhiteTexture = std::make_shared<Texture2D>(1, 1);
    s_Data.TextureSlots[0] = s_Data.WhiteTexture;

    // Shader
    s_Data.QuadShader = std::make_unique<Shader>(
        "assets/shaders/quad.vert", "assets/shaders/quad.frag");
    s_Data.QuadShader->Bind();

    // Set sampler array uniform: u_Textures[0] = 0, u_Textures[1] = 1, ...
    int samplers[MaxTextureSlots];
    for (uint32_t i = 0; i < MaxTextureSlots; i++)
        samplers[i] = static_cast<int>(i);
    s_Data.QuadShader->SetIntArray("u_Textures", samplers, MaxTextureSlots);

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer2D::Shutdown() {
    delete[] s_Data.VertexBufferBase;
    s_Data.VertexBufferBase = nullptr;

    // Release smart pointers BEFORE the OpenGL context is destroyed,
    // so GPU resources (textures, shaders) are freed while the context
    // is still valid. Without this, static destruction order could cause
    // GPU driver errors at program exit.
    s_Data.QuadShader.reset();
 for(auto& t:s_Data.TextureSlots)t.reset();
    s_Data.WhiteTexture.reset();
    for (auto& slot : s_Data.TextureSlots)
        slot.reset();

    glDeleteVertexArrays(1, &s_Data.VAO);
    glDeleteBuffers(1, &s_Data.VBO);
    glDeleteBuffers(1, &s_Data.EBO);
}

// ── Batch lifecycle ────────────────────────────────────────────────────────────
void Renderer2D::BeginBatch(const OrthographicCamera& camera) {
    s_Data.QuadShader->Bind();
    s_Data.QuadShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
    StartNewBatch();
}

void Renderer2D::StartNewBatch() {
    s_Data.QuadCount = 0;
    s_Data.VertexBufferPtr = s_Data.VertexBufferBase;
    s_Data.TextureSlotIndex = 1;  // slot 0 is always the white texture
}

void Renderer2D::EndBatch() {
    if (s_Data.QuadCount == 0)
        return;

    uint32_t dataSize = static_cast<uint32_t>(
        reinterpret_cast<uint8_t*>(s_Data.VertexBufferPtr) -
        reinterpret_cast<uint8_t*>(s_Data.VertexBufferBase));

    glBindBuffer(GL_ARRAY_BUFFER, s_Data.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, s_Data.VertexBufferBase);

    Flush();
}

void Renderer2D::Flush() {
    // Bind all used textures
    for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++) {
        s_Data.TextureSlots[i]->Bind(i);
    }

    glBindVertexArray(s_Data.VAO);
    glDrawElements(GL_TRIANGLES, s_Data.QuadCount * 6,
                   GL_UNSIGNED_INT, nullptr);

    s_Data.Stats.DrawCalls++;
}

// ── Draw calls ─────────────────────────────────────────────────────────────────
void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                           const glm::vec4& color)
{
    DrawQuad(position, size, 0.0f, nullptr, color);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                           const std::shared_ptr<Texture2D>& texture,
                           const glm::vec4& tintColor)
{
    DrawQuad(position, size, 0.0f, texture, tintColor);
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size,
                           float rotation,
                           const std::shared_ptr<Texture2D>& texture,
                           const glm::vec4& tintColor,
                           const glm::vec4& uvRect)
{
    // Auto-flush if batch is full
    if (s_Data.QuadCount >= MaxQuads) {
        EndBatch();
        StartNewBatch();
    }

    float texIndex = 0.0f;  // default: white texture (slot 0)
    if (texture) {
        texIndex = GetTextureIndex(texture);
    }

    // Compute the 4 corner positions
    glm::mat4 transform = glm::translate(glm::mat4(1.0f),
                                          glm::vec3(position, 0.0f));
    if (rotation != 0.0f) {
        transform = glm::rotate(transform, glm::radians(rotation),
                                glm::vec3(0.0f, 0.0f, 1.0f));
    }
    transform = glm::scale(transform, glm::vec3(size, 1.0f));

    // UV coordinates from the uvRect (for atlas sub-regions)
    float uvLeft   = uvRect.x;
    float uvBottom = uvRect.y;
    float uvRight  = uvRect.z;
    float uvTop    = uvRect.w;

    // Quad corners (centered at origin, scaled by transform)
    glm::vec4 positions[4] = {
        transform * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f),  // bottom-left
        transform * glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f),  // bottom-right
        transform * glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f),  // top-right
        transform * glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f),  // top-left
    };

    glm::vec2 texCoords[4] = {
        {uvLeft,  uvBottom},
        {uvRight, uvBottom},
        {uvRight, uvTop},
        {uvLeft,  uvTop},
    };

    for (int i = 0; i < 4; i++) {
        s_Data.VertexBufferPtr->Position  = glm::vec3(positions[i]);
        s_Data.VertexBufferPtr->Color     = tintColor;
        s_Data.VertexBufferPtr->TexCoords = texCoords[i];
        s_Data.VertexBufferPtr->TexIndex  = texIndex;
        s_Data.VertexBufferPtr++;
    }

    s_Data.QuadCount++;
    s_Data.Stats.QuadCount++;
}

float Renderer2D::GetTextureIndex(const std::shared_ptr<Texture2D>& texture) {
    // Check if texture is already bound
    for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
        if (*s_Data.TextureSlots[i] == *texture) {
            return static_cast<float>(i);
        }
    }

    // Need a new slot — flush if all slots are full
    if (s_Data.TextureSlotIndex >= MaxTextureSlots) {
        EndBatch();
        StartNewBatch();
    }

    float index = static_cast<float>(s_Data.TextureSlotIndex);
    s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
    s_Data.TextureSlotIndex++;
    return index;
}

Renderer2DStats Renderer2D::GetStats() {
    return s_Data.Stats;
}

void Renderer2D::ResetStats() {
    s_Data.Stats.DrawCalls = 0;
    s_Data.Stats.QuadCount = 0;
}
