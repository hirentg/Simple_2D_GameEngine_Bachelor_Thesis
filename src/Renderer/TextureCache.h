#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "Texture2D.h"

// Simple cache that prevents redundant GPU texture uploads.
// Loading the same file path twice returns the existing GPU texture.
class TextureCache {
public:
    std::shared_ptr<Texture2D> Load(const std::string& filepath) {
        auto it = m_Cache.find(filepath);
        if (it != m_Cache.end()) {
            return it->second;
        }
        auto texture = std::make_shared<Texture2D>(filepath);
        m_Cache[filepath] = texture;
        return texture;
    }

    void Clear() {
        m_Cache.clear();
    }

    size_t Size() const { return m_Cache.size(); }

private:
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_Cache;
};
