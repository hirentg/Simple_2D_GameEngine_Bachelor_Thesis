#pragma once

#include <string>
#include <glad/glad.h>

class Texture2D {
public:
    Texture2D(const std::string& filepath);
    Texture2D(uint32_t width, uint32_t height);  // create blank white texture
    ~Texture2D();

    // Non-copyable
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    void Bind(uint32_t slot = 0) const;

    uint32_t GetWidth()  const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    GLuint   GetID()     const { return m_RendererID; }

    bool operator==(const Texture2D& other) const {
        return m_RendererID == other.m_RendererID;
    }

private:
    GLuint   m_RendererID = 0;
    uint32_t m_Width  = 0;
    uint32_t m_Height = 0;
};
