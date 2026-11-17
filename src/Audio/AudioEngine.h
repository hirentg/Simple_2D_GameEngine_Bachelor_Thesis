#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>
#include <memory>

// Forward-declare miniaudio types to keep the header lightweight.
// The full miniaudio.h is only included in AudioEngine.cpp.
#include <miniaudio.h>
#ifdef PlaySound
#undef PlaySound
#endif

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    // Non-copyable, non-movable
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    // Play a sound. If loop=true, the sound repeats until stopped.
    // Returns a handle ID that can be used to stop the specific sound.
    uint32_t PlaySound(const std::string& filepath, float volume = 1.0f,
                       bool loop = false);

    // Stop a specific sound by handle
    void StopSound(uint32_t handle);

    // Stop all currently playing sounds
    void StopAll();
 void CollectFinished();

    // Set master volume [0.0, 1.0]
    void SetMasterVolume(float volume);

private:
    std::unique_ptr<ma_engine> m_Engine;
    std::unordered_map<uint32_t, std::unique_ptr<ma_sound>> m_ActiveSounds;
    uint32_t m_NextHandle = 1;
};
